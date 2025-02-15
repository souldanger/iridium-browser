// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/browser/renderer_host/pepper/pepper_socket_utils.h"

#include <string>
#include <vector>

#include "base/logging.h"
#include "base/memory/ptr_util.h"
#include "base/memory/ref_counted.h"
#include "base/strings/string_util.h"
#include "build/build_config.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/browser/content_browser_client.h"
#include "content/public/browser/render_frame_host.h"
#include "content/public/browser/site_instance.h"
#include "content/public/common/content_client.h"
#include "net/base/ip_address.h"
#include "net/base/ip_endpoint.h"
#include "net/cert/x509_certificate.h"
#include "ppapi/c/private/ppb_net_address_private.h"
#include "ppapi/shared_impl/private/net_address_private_impl.h"
#include "ppapi/shared_impl/private/ppb_x509_certificate_private_shared.h"

#if defined(OS_CHROMEOS)
#include "chromeos/network/firewall_hole.h"
#endif  // defined(OS_CHROMEOS)

namespace content {
namespace pepper_socket_utils {

SocketPermissionRequest CreateSocketPermissionRequest(
    SocketPermissionRequest::OperationType type,
    const PP_NetAddress_Private& net_addr) {
  std::string host =
      ppapi::NetAddressPrivateImpl::DescribeNetAddress(net_addr, false);
  uint16_t port = 0;
  std::vector<unsigned char> address;
  ppapi::NetAddressPrivateImpl::NetAddressToIPEndPoint(
      net_addr, &address, &port);
  return SocketPermissionRequest(type, host, port);
}

bool CanUseSocketAPIs(bool external_plugin,
                      bool private_api,
                      const SocketPermissionRequest* params,
                      int render_process_id,
                      int render_frame_id) {
  DCHECK_CURRENTLY_ON(BrowserThread::UI);
  if (!external_plugin) {
    // Always allow socket APIs for out-process plugins (other than external
    // plugins instantiated by the embedder through
    // BrowserPpapiHost::CreateExternalPluginProcess).
    return true;
  }

  RenderFrameHost* render_frame_host =
      RenderFrameHost::FromID(render_process_id, render_frame_id);
  if (!render_frame_host)
    return false;
  SiteInstance* site_instance = render_frame_host->GetSiteInstance();
  if (!site_instance)
    return false;
  if (!GetContentClient()->browser()->AllowPepperSocketAPI(
          site_instance->GetBrowserContext(),
          site_instance->GetSiteURL(),
          private_api,
          params)) {
    LOG(ERROR) << "Host " << site_instance->GetSiteURL().host()
               << " cannot use socket API or destination is not allowed";
    return false;
  }

  return true;
}

bool GetCertificateFields(const net::X509Certificate& cert,
                          ppapi::PPB_X509Certificate_Fields* fields) {
  const net::CertPrincipal& issuer = cert.issuer();
  fields->SetField(PP_X509CERTIFICATE_PRIVATE_ISSUER_COMMON_NAME,
                   base::MakeUnique<base::StringValue>(issuer.common_name));
  fields->SetField(PP_X509CERTIFICATE_PRIVATE_ISSUER_LOCALITY_NAME,
                   base::MakeUnique<base::StringValue>(issuer.locality_name));
  fields->SetField(
      PP_X509CERTIFICATE_PRIVATE_ISSUER_STATE_OR_PROVINCE_NAME,
      base::MakeUnique<base::StringValue>(issuer.state_or_province_name));
  fields->SetField(PP_X509CERTIFICATE_PRIVATE_ISSUER_COUNTRY_NAME,
                   base::MakeUnique<base::StringValue>(issuer.country_name));
  fields->SetField(PP_X509CERTIFICATE_PRIVATE_ISSUER_ORGANIZATION_NAME,
                   base::MakeUnique<base::StringValue>(
                       base::JoinString(issuer.organization_names, "\n")));
  fields->SetField(PP_X509CERTIFICATE_PRIVATE_ISSUER_ORGANIZATION_UNIT_NAME,
                   base::MakeUnique<base::StringValue>(
                       base::JoinString(issuer.organization_unit_names, "\n")));

  const net::CertPrincipal& subject = cert.subject();
  fields->SetField(PP_X509CERTIFICATE_PRIVATE_SUBJECT_COMMON_NAME,
                   base::MakeUnique<base::StringValue>(subject.common_name));
  fields->SetField(PP_X509CERTIFICATE_PRIVATE_SUBJECT_LOCALITY_NAME,
                   base::MakeUnique<base::StringValue>(subject.locality_name));
  fields->SetField(
      PP_X509CERTIFICATE_PRIVATE_SUBJECT_STATE_OR_PROVINCE_NAME,
      base::MakeUnique<base::StringValue>(subject.state_or_province_name));
  fields->SetField(PP_X509CERTIFICATE_PRIVATE_SUBJECT_COUNTRY_NAME,
                   base::MakeUnique<base::StringValue>(subject.country_name));
  fields->SetField(PP_X509CERTIFICATE_PRIVATE_SUBJECT_ORGANIZATION_NAME,
                   base::MakeUnique<base::StringValue>(
                       base::JoinString(subject.organization_names, "\n")));
  fields->SetField(PP_X509CERTIFICATE_PRIVATE_SUBJECT_ORGANIZATION_UNIT_NAME,
                   base::MakeUnique<base::StringValue>(base::JoinString(
                       subject.organization_unit_names, "\n")));

  const std::string& serial_number = cert.serial_number();
  fields->SetField(PP_X509CERTIFICATE_PRIVATE_SERIAL_NUMBER,
                   base::BinaryValue::CreateWithCopiedBuffer(
                       serial_number.data(), serial_number.length()));
  fields->SetField(
      PP_X509CERTIFICATE_PRIVATE_VALIDITY_NOT_BEFORE,
      base::MakeUnique<base::Value>(cert.valid_start().ToDoubleT()));
  fields->SetField(
      PP_X509CERTIFICATE_PRIVATE_VALIDITY_NOT_AFTER,
      base::MakeUnique<base::Value>(cert.valid_expiry().ToDoubleT()));
  std::string der;
  net::X509Certificate::GetDEREncoded(cert.os_cert_handle(), &der);
  fields->SetField(
      PP_X509CERTIFICATE_PRIVATE_RAW,
      base::BinaryValue::CreateWithCopiedBuffer(der.data(), der.length()));
  return true;
}

bool GetCertificateFields(const char* der,
                          uint32_t length,
                          ppapi::PPB_X509Certificate_Fields* fields) {
  scoped_refptr<net::X509Certificate> cert =
      net::X509Certificate::CreateFromBytes(der, length);
  if (!cert.get())
    return false;
  return GetCertificateFields(*cert.get(), fields);
}

#if defined(OS_CHROMEOS)
namespace {

// The entire IPv4 subnet 127.0.0.0/8 is for loopback. See RFC3330.
const uint8_t kIPv4LocalhostPrefix[] = {127};

bool IsLoopbackAddress(const net::IPAddress& address) {
  if (address.IsIPv4()) {
    return net::IPAddressStartsWith(address, kIPv4LocalhostPrefix);
  } else if (address.IsIPv6()) {
    // ::1 is the only loopback address in ipv6.
    return address == net::IPAddress::IPv6Localhost();
  }
  return false;
}

}  // namespace

void OpenFirewallHole(const net::IPEndPoint& address,
                      chromeos::FirewallHole::PortType type,
                      FirewallHoleOpenCallback callback) {
  if (IsLoopbackAddress(address.address())) {
    callback.Run(nullptr);
    return;
  }

  // TODO(sergeyu): Currently an empty string is passed as interface name, which
  // means the port will be opened on all network interfaces. Interface name
  // can be resolved by the address, but the best solution would be to update
  // firewalld to allow filtering by destination address, not just destination
  // port. iptables already support it.
  chromeos::FirewallHole::Open(type, address.port(), std::string(), callback);
}

void OpenTCPFirewallHole(const net::IPEndPoint& address,
                         FirewallHoleOpenCallback callback) {
  OpenFirewallHole(address, chromeos::FirewallHole::PortType::TCP, callback);
}

void OpenUDPFirewallHole(const net::IPEndPoint& address,
                         FirewallHoleOpenCallback callback) {
  OpenFirewallHole(address, chromeos::FirewallHole::PortType::UDP, callback);
}
#endif  // defined(OS_CHROMEOS)

}  // namespace pepper_socket_utils
}  // namespace content
