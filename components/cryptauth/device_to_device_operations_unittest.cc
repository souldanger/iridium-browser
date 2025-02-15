// Copyright 2015 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/base64url.h"
#include "base/bind.h"
#include "base/macros.h"
#include "components/cryptauth/device_to_device_initiator_operations.h"
#include "components/cryptauth/device_to_device_responder_operations.h"
#include "components/cryptauth/fake_secure_message_delegate.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace cryptauth {

namespace {

// The initiator's session public key in base64url form. Note that this is
// actually a serialized proto.
const char kInitiatorSessionPublicKeyBase64[] =
    "CAESRQogOlH8DgPMQu7eAt-b6yoTXcazG8mAl6SPC5Ds-LTULIcSIQDZDMqsoYRO4tNMej1FB"
    "El1sTiTiVDqrcGq-CkYCzDThw==";

// The responder's session public key in base64url form. Note that this is
// actually a serialized proto.
const char kResponderSessionPublicKeyBase64[] =
    "CAESRgohAN9QYU5HySO14Gi9PDIClacBnC0C8wqPwXsNHUNG_vXlEiEAggzU80ZOd9DWuCBdp"
    "6bzpGcC-oj1yrwdVCHGg_yeaAQ=";

// The long-term public key possessed by the responder device.
const char kResponderPersistentPublicKey[] = "responder persistent public key";

// Used as a callback for message creation operations to save |message| into
// |out_message|.
void SaveMessageResult(std::string* out_message, const std::string& message) {
  *out_message = message;
}

// Used as a callback for validation operations to save |success| and into
// |out_success|.
void SaveValidationResult(bool* out_success, bool success) {
  *out_success = success;
}

// Used as a callback for the ValidateResponderAuthMessage and
// ValidateHelloMessage operations, saving both the outcome and the returned
// key.
void SaveValidationResultWithKey(bool* out_success,
                                 std::string* out_key,
                                 bool success,
                                 const std::string& key) {
  *out_success = success;
  *out_key = key;
}

}  // namespace

class ProximityAuthDeviceToDeviceOperationsTest : public testing::Test {
 protected:
  ProximityAuthDeviceToDeviceOperationsTest() {}
  ~ProximityAuthDeviceToDeviceOperationsTest() override {}

  void SetUp() override {
    ASSERT_TRUE(
        base::Base64UrlDecode(kInitiatorSessionPublicKeyBase64,
                              base::Base64UrlDecodePolicy::REQUIRE_PADDING,
                              &local_session_public_key_));
    local_session_private_key_ =
        secure_message_delegate_.GetPrivateKeyForPublicKey(
            local_session_public_key_);

    ASSERT_TRUE(
        base::Base64UrlDecode(kResponderSessionPublicKeyBase64,
                              base::Base64UrlDecodePolicy::REQUIRE_PADDING,
                              &remote_session_public_key_));
    remote_session_private_key_ =
        secure_message_delegate_.GetPrivateKeyForPublicKey(
            remote_session_public_key_);

    // Note: FakeSecureMessageDelegate functions are synchronous.
    secure_message_delegate_.DeriveKey(
        local_session_private_key_, remote_session_public_key_,
        base::Bind(&SaveMessageResult, &session_symmetric_key_));

    persistent_symmetric_key_ = "persistent symmetric key";
  }

  // Creates the initator's [Hello] message.
  std::string CreateHelloMessage() {
    std::string hello_message;
    DeviceToDeviceInitiatorOperations::CreateHelloMessage(
        local_session_public_key_, persistent_symmetric_key_,
        &secure_message_delegate_,
        base::Bind(&SaveMessageResult, &hello_message));
    EXPECT_FALSE(hello_message.empty());
    return hello_message;
  }

  // Creates the responder's [Remote Auth] message.
  std::string CreateResponderAuthMessage(const std::string& hello_message) {
    std::string persistent_responder_private_key =
        secure_message_delegate_.GetPrivateKeyForPublicKey(
            kResponderPersistentPublicKey);

    std::string remote_auth_message;
    DeviceToDeviceResponderOperations::CreateResponderAuthMessage(
        hello_message, remote_session_public_key_, remote_session_private_key_,
        persistent_responder_private_key, persistent_symmetric_key_,
        &secure_message_delegate_,
        base::Bind(&SaveMessageResult, &remote_auth_message));
    EXPECT_FALSE(remote_auth_message.empty());
    return remote_auth_message;
  }

  // Creates the initiator's [Initiator Auth] message.
  std::string CreateInitiatorAuthMessage(
      const std::string& remote_auth_message) {
    std::string local_auth_message;
    DeviceToDeviceInitiatorOperations::CreateInitiatorAuthMessage(
        session_symmetric_key_, persistent_symmetric_key_, remote_auth_message,
        &secure_message_delegate_,
        base::Bind(&SaveMessageResult, &local_auth_message));
    EXPECT_FALSE(local_auth_message.empty());
    return local_auth_message;
  }

  FakeSecureMessageDelegate secure_message_delegate_;

  std::string persistent_symmetric_key_;
  std::string local_session_public_key_;
  std::string local_session_private_key_;
  std::string remote_session_public_key_;
  std::string remote_session_private_key_;
  std::string session_symmetric_key_;

  DISALLOW_COPY_AND_ASSIGN(ProximityAuthDeviceToDeviceOperationsTest);
};

TEST_F(ProximityAuthDeviceToDeviceOperationsTest,
       ValidateHelloMessage_Success) {
  bool validation_success = false;
  std::string hello_public_key;
  DeviceToDeviceResponderOperations::ValidateHelloMessage(
      CreateHelloMessage(), persistent_symmetric_key_,
      &secure_message_delegate_,
      base::Bind(&SaveValidationResultWithKey, &validation_success,
                 &hello_public_key));

  EXPECT_TRUE(validation_success);
  EXPECT_EQ(local_session_public_key_, hello_public_key);
}

TEST_F(ProximityAuthDeviceToDeviceOperationsTest,
       ValidateHelloMessage_Failure) {
  bool validation_success = true;
  std::string hello_public_key = "non-empty string";
  DeviceToDeviceResponderOperations::ValidateHelloMessage(
      "some random string", persistent_symmetric_key_,
      &secure_message_delegate_,
      base::Bind(&SaveValidationResultWithKey, &validation_success,
                 &hello_public_key));

  EXPECT_FALSE(validation_success);
  EXPECT_TRUE(hello_public_key.empty());
}

TEST_F(ProximityAuthDeviceToDeviceOperationsTest,
       ValidateResponderAuthMessage_Success) {
  std::string hello_message = CreateHelloMessage();
  std::string remote_auth_message = CreateResponderAuthMessage(hello_message);

  bool validation_success = false;
  std::string session_symmetric_key;
  DeviceToDeviceInitiatorOperations::ValidateResponderAuthMessage(
      remote_auth_message, kResponderPersistentPublicKey,
      persistent_symmetric_key_, local_session_private_key_, hello_message,
      &secure_message_delegate_,
      base::Bind(&SaveValidationResultWithKey, &validation_success,
                 &session_symmetric_key));

  EXPECT_TRUE(validation_success);
  EXPECT_EQ(session_symmetric_key_, session_symmetric_key);
}

TEST_F(ProximityAuthDeviceToDeviceOperationsTest,
       ValidateResponderAuthMessage_InvalidHelloMessage) {
  std::string hello_message = CreateHelloMessage();
  std::string remote_auth_message = CreateResponderAuthMessage(hello_message);

  bool validation_success = true;
  std::string session_symmetric_key = "non empty";
  DeviceToDeviceInitiatorOperations::ValidateResponderAuthMessage(
      remote_auth_message, kResponderPersistentPublicKey,
      persistent_symmetric_key_, local_session_private_key_,
      "invalid hello message", &secure_message_delegate_,
      base::Bind(&SaveValidationResultWithKey, &validation_success,
                 &session_symmetric_key));

  EXPECT_FALSE(validation_success);
  EXPECT_TRUE(session_symmetric_key.empty());
}

TEST_F(ProximityAuthDeviceToDeviceOperationsTest,
       ValidateResponderAuthMessage_InvalidPSK) {
  std::string hello_message = CreateHelloMessage();
  std::string remote_auth_message = CreateResponderAuthMessage(hello_message);

  bool validation_success = true;
  std::string session_symmetric_key = "non empty";
  DeviceToDeviceInitiatorOperations::ValidateResponderAuthMessage(
      remote_auth_message, kResponderPersistentPublicKey,
      "invalid persistent symmetric key", local_session_private_key_,
      hello_message, &secure_message_delegate_,
      base::Bind(&SaveValidationResultWithKey, &validation_success,
                 &session_symmetric_key));

  EXPECT_FALSE(validation_success);
  EXPECT_TRUE(session_symmetric_key.empty());
}

TEST_F(ProximityAuthDeviceToDeviceOperationsTest,
       ValidateInitiatorAuthMessage_Success) {
  std::string hello_message = CreateHelloMessage();
  std::string remote_auth_message = CreateResponderAuthMessage(hello_message);
  std::string local_auth_message =
      CreateInitiatorAuthMessage(remote_auth_message);

  bool validation_success = false;
  DeviceToDeviceResponderOperations::ValidateInitiatorAuthMessage(
      local_auth_message, session_symmetric_key_, persistent_symmetric_key_,
      remote_auth_message, &secure_message_delegate_,
      base::Bind(&SaveValidationResult, &validation_success));

  EXPECT_TRUE(validation_success);
}

TEST_F(ProximityAuthDeviceToDeviceOperationsTest,
       ValidateInitiatorAuthMessage_InvalidRemoteAuth) {
  std::string hello_message = CreateHelloMessage();
  std::string remote_auth_message = CreateResponderAuthMessage(hello_message);
  std::string local_auth_message =
      CreateInitiatorAuthMessage(remote_auth_message);

  bool validation_success = true;
  DeviceToDeviceResponderOperations::ValidateInitiatorAuthMessage(
      local_auth_message, session_symmetric_key_, persistent_symmetric_key_,
      "invalid remote auth", &secure_message_delegate_,
      base::Bind(&SaveValidationResult, &validation_success));

  EXPECT_FALSE(validation_success);
}

TEST_F(ProximityAuthDeviceToDeviceOperationsTest,
       ValidateInitiatorAuthMessage_InvalidPSK) {
  std::string hello_message = CreateHelloMessage();
  std::string remote_auth_message = CreateResponderAuthMessage(hello_message);
  std::string local_auth_message =
      CreateInitiatorAuthMessage(remote_auth_message);

  bool validation_success = true;
  DeviceToDeviceResponderOperations::ValidateInitiatorAuthMessage(
      local_auth_message, session_symmetric_key_,
      "invalid persistent symmetric key", remote_auth_message,
      &secure_message_delegate_,
      base::Bind(&SaveValidationResult, &validation_success));

  EXPECT_FALSE(validation_success);
}

}  // namespace cryptauth
