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

#ifndef ADDON_ZIGBEE_ZIGBEE_H_
#define ADDON_ZIGBEE_ZIGBEE_H_

#include <node.h>
#include <node_object_wrap.h>
#include <uv.h>
#include <artik_zigbee.hh>

#include <loop.h>

using v8::Function;
using v8::Local;
using v8::Isolate;

namespace artik {

class ZigbeeWrapper: public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  Zigbee* getObj() { return m_zb; }

  v8::Persistent<v8::Function>* getIintCb() { return m_init_cb; }

 private:
  ZigbeeWrapper();
  ~ZigbeeWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;
  static void initialize(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void network_start(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void network_form(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void network_form_manually(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void network_permitjoin(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void network_leave(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void network_join(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void network_find(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void network_request_my_network_status(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void device_request_my_node_type(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void device_find_by_cluster(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void reset_local(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void network_stop_scan(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void network_join_manually(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void device_discover(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void set_discover_cycle_time(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_discovered_device_list(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void raw_request(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_local_device_list(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  Zigbee* m_zb;
  v8::Persistent<v8::Function>* m_init_cb;
  GlibLoop* m_loop;
};

}  // namespace artik

#endif  // ADDON_ZIGBEE_ZIGBEE_H_
