// Copyright 2016 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONTENT_SHELL_TEST_RUNNER_WEB_VIEW_TEST_CLIENT_H_
#define CONTENT_SHELL_TEST_RUNNER_WEB_VIEW_TEST_CLIENT_H_

#include "base/macros.h"
#include "third_party/WebKit/public/web/WebViewClient.h"

namespace blink {
class WebView;
}  // namespace blink

namespace test_runner {

class TestRunner;
class WebTestDelegate;
class WebViewTestProxyBase;

// WebViewTestClient implements WebViewClient interface, providing behavior
// expected by tests.  WebViewTestClient ends up used by WebViewTestProxy
// which coordinates forwarding WebViewClient calls either to
// WebViewTestClient or to the product code (i.e. to RenderViewImpl).
class WebViewTestClient : public blink::WebViewClient {
 public:
  // Caller has to ensure |web_view_test_proxy_base| lives longer than |this|.
  WebViewTestClient(WebViewTestProxyBase* web_view_test_proxy_base);

  virtual ~WebViewTestClient();

  // WebViewClient overrides needed by WebViewTestProxy.
  void showValidationMessage(const blink::WebRect& anchor_in_root_view,
                             const blink::WebString& main_message,
                             blink::WebTextDirection main_message_hint,
                             const blink::WebString& sub_message,
                             blink::WebTextDirection sub_message_hint) override;
  void didChangeContents() override;
  blink::WebView* createView(blink::WebLocalFrame* creator,
                             const blink::WebURLRequest& request,
                             const blink::WebWindowFeatures& features,
                             const blink::WebString& frame_name,
                             blink::WebNavigationPolicy policy,
                             bool suppress_opener) override;
  void setStatusText(const blink::WebString& text) override;
  void printPage(blink::WebLocalFrame* frame) override;
  blink::WebSpeechRecognizer* speechRecognizer() override;
  blink::WebString acceptLanguages() override;
  void didFocus() override;

 private:
  WebTestDelegate* delegate();
  TestRunner* test_runner();

  // Borrowed pointer to WebViewTestProxyBase.
  WebViewTestProxyBase* web_view_test_proxy_base_;

  DISALLOW_COPY_AND_ASSIGN(WebViewTestClient);
};

}  // namespace test_runner

#endif  // CONTENT_SHELL_TEST_RUNNER_WEB_VIEW_TEST_CLIENT_H_
