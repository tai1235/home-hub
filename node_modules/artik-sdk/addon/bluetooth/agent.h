/*
 *
 * Copyright 2017 Samsung Electronics All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the License.
 *
 */

#ifndef ADDON_BLUETOOTH_AGENT_H_
#define ADDON_BLUETOOTH_AGENT_H_

#include <node.h>
#include <node_object_wrap.h>

#include <artik_bluetooth.hh>
#include <loop.h>

#include <array>

namespace artik {

class AgentRequestWrapper : public node::ObjectWrap {
 public:
  ~AgentRequestWrapper();

  static void Init(v8::Local<v8::Object> exports);
  static v8::Local<v8::Object> newInstance(
      artik_bt_agent_request_handle handle);

  Bluetooth* getObj() { return m_bt; }
  void setRequestHandle(artik_bt_agent_request_handle handle);

 private:
  AgentRequestWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;
  static std::array<const char *, 2> error_types;

  static void agent_send_pincode(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void agent_send_passkey(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void agent_send_error(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void agent_send_empty_response(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  Bluetooth* m_bt;
  artik_bt_agent_request_handle m_handle;
};

class AgentWrapper : public node::ObjectWrap {
 public:
  ~AgentWrapper();

  static void Init(v8::Local<v8::Object> exports);

  Bluetooth* getObj() { return m_bt; }

  v8::Persistent<v8::Function>* getReleaseCb() {
    return m_release_cb;
  }
  v8::Persistent<v8::Function>* getRequestPincodeCb() {
    return m_request_pincode_cb;
  }
  v8::Persistent<v8::Function>* getDisplayPincodeCb() {
    return m_display_pincode_cb;
  }
  v8::Persistent<v8::Function>* getRequestPasskeyCb() {
    return m_request_passkey_cb;
  }
  v8::Persistent<v8::Function>* getDisplayPasskeyCb() {
    return m_display_passkey_cb;
  }
  v8::Persistent<v8::Function>* getRequestConfirmationCb() {
    return m_request_confirmation_cb;
  }
  v8::Persistent<v8::Function>* getRequestAuthorizationCb() {
    return m_request_authorization_cb;
  }
  v8::Persistent<v8::Function>* getAuthorizeServiceCb() {
    return m_authorize_service_cb;
  }
  v8::Persistent<v8::Function>* getCancelCb() {
    return m_cancel_cb;
  }

 private:
  AgentWrapper(
      v8::Persistent<v8::Function> *release_cb,
      v8::Persistent<v8::Function> *request_pincode_cb,
      v8::Persistent<v8::Function> *display_pincode_cb,
      v8::Persistent<v8::Function> *request_passkey_cb,
      v8::Persistent<v8::Function> *display_passkey_cb,
      v8::Persistent<v8::Function> *request_confirmation_cb,
      v8::Persistent<v8::Function> *request_authorization_cb,
      v8::Persistent<v8::Function> *authorize_service_cb,
      v8::Persistent<v8::Function> *cancel_cb);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static v8::Persistent<v8::Function> constructor;
  static std::array<const char*, 5> capabilities;

  static void agent_register_capability(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void agent_set_default(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void agent_unregister(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  Bluetooth* m_bt;

  v8::Persistent<v8::Function> *m_release_cb;
  v8::Persistent<v8::Function> *m_request_pincode_cb;
  v8::Persistent<v8::Function> *m_display_pincode_cb;
  v8::Persistent<v8::Function> *m_request_passkey_cb;
  v8::Persistent<v8::Function> *m_display_passkey_cb;
  v8::Persistent<v8::Function> *m_request_confirmation_cb;
  v8::Persistent<v8::Function> *m_request_authorization_cb;
  v8::Persistent<v8::Function> *m_authorize_service_cb;
  v8::Persistent<v8::Function> *m_cancel_cb;

  GlibLoop *m_loop;
};

}  // namespace artik

#endif  // ADDON_BLUETOOTH_AGENT_H_

