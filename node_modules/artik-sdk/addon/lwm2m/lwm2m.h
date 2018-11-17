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

#ifndef ADDON_LWM2M_LWM2M_H_
#define ADDON_LWM2M_LWM2M_H_

#include <node.h>
#include <node_object_wrap.h>

#include <uv.h>
#include <artik_lwm2m.hh>

#include <loop.h>

using v8::Function;
using v8::Local;

namespace artik {

class Lwm2mWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  Lwm2m* getObj() { return m_lwm2m; }
  v8::Persistent<v8::Function>* getErrorCb() { return m_error_cb; }
  v8::Persistent<v8::Function>* getExecuteCb() { return m_execute_cb; }
  v8::Persistent<v8::Function>* getChangedCb() { return m_changed_cb; }

 private:
  Lwm2mWrapper();
  ~Lwm2mWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void client_request(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void client_release(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void client_connect(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void client_disconnect(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void client_write_resource(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void client_read_resource(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void serialize_tlv_int(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void serialize_tlv_string(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  Lwm2m *m_lwm2m;
  artik_lwm2m_config m_config;
  v8::Persistent<v8::Function>* m_error_cb;
  v8::Persistent<v8::Function>* m_execute_cb;
  v8::Persistent<v8::Function>* m_changed_cb;
  GlibLoop* m_loop;
};

}  // namespace artik


#endif  // ADDON_LWM2M_LWM2M_H_
