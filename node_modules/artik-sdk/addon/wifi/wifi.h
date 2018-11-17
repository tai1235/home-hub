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

#ifndef ADDON_WIFI_WIFI_H_
#define ADDON_WIFI_WIFI_H_

#include <node.h>
#include <node_object_wrap.h>

#include <uv.h>
#include <artik_wifi.hh>

#include <utils.h>
#include <loop.h>

namespace artik {

class WifiWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  Wifi* getObj() { return m_wifi; }
  artik_wifi_mode_t getMode() { return m_wifi_mode; }
  void setMode(artik_wifi_mode_t wifi_mode) { m_wifi_mode = wifi_mode; }
  v8::Persistent<v8::Function>* getScanCb() { return m_scan_cb; }
  v8::Persistent<v8::Function>* getConnectCb() { return m_connect_cb; }

 private:
  WifiWrapper();
  ~WifiWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void disconnect(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void scan_request(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_scan_result(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_info(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void connect(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void start_ap(const v8::FunctionCallbackInfo<v8::Value>& args);

  Wifi* m_wifi;
  artik_wifi_mode_t m_wifi_mode;

  v8::Persistent<v8::Function>* m_scan_cb;
  v8::Persistent<v8::Function>* m_connect_cb;
  GlibLoop* m_loop;
};

}  // namespace artik

#endif  // ADDON_WIFI_WIFI_H_
