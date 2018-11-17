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

#ifndef ADDON_SECURITY_SECURITY_H_
#define ADDON_SECURITY_SECURITY_H_

#include <node.h>
#include <nan.h>
#include <node_object_wrap.h>

#include <artik_security.hh>

namespace artik {

class SecurityWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  Security* getObj() { return m_security; }

 private:
  SecurityWrapper();
  ~SecurityWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void get_certificate(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_ca_chain(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_key_from_cert(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_random_bytes(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_certificate_sn(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void verify_signature_init(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void verify_signature_update(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void verify_signature_final(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  Security * m_security;
};

}  // namespace artik

#endif  // ADDON_SECURITY_SECURITY_H_
