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

#ifndef ADDON_BLUETOOTH_BLUETOOTH_H_
#define ADDON_BLUETOOTH_BLUETOOTH_H_

#include <node.h>
#include <node_object_wrap.h>

#include <uv.h>
#include <artik_bluetooth.hh>
#include <loop.h>

#include <array>
#include <unordered_map>

using v8::Function;
using v8::Local;

namespace artik {

class BluetoothWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  Bluetooth* getObj() { return m_bt; }
  v8::Persistent<v8::Function>* getScanCb() { return m_scan_cb; }
  v8::Persistent<v8::Function>* getBondCb() { return m_bond_cb; }
  v8::Persistent<v8::Function>* getConnectCb() { return m_connect_cb; }

 private:
  BluetoothWrapper();
  ~BluetoothWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static std::array<const char*, 3> bt_scan_types;
  static std::array<const char*, 3> bt_device_types;
  static std::array<const char*, 6> bt_device_properties;
  static std::unordered_map<int, const char*> bt_major_device_class;
  static std::unordered_map<int, const char*> bt_minor_device_class;
  static std::unordered_map<int, const char*> bt_device_service_class;

  static void start_scan(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void stop_scan(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_devices(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void start_bond(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void stop_bond(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void connect(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void disconnect(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void remove_unpaired_devices(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void remove_device(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void set_scan_filter(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void set_alias(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void set_discoverable(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void set_pairable(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void set_pairableTimeout(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void set_discoverableTimeout(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void is_scanning(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_adapter_info(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void remove_devices(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void connect_profile(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void set_trust(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void unset_trust(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void set_block(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void unset_block(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void is_paired(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void is_connected(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void is_trusted(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void is_blocked(const v8::FunctionCallbackInfo<v8::Value>& args);

  Bluetooth* m_bt;
  v8::Persistent<v8::Function>* m_scan_cb;
  v8::Persistent<v8::Function>* m_bond_cb;
  v8::Persistent<v8::Function>* m_connect_cb;
  GlibLoop *m_loop;
};

}  // namespace artik

#endif  // ADDON_BLUETOOTH_BLUETOOTH_H_

