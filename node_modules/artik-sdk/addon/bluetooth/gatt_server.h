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

#ifndef ADDON_BLUETOOTH_GATT_SERVER_H_
#define ADDON_BLUETOOTH_GATT_SERVER_H_

#include <node.h>
#include <node_object_wrap.h>

#include <artik_bluetooth.hh>
#include <loop.h>

#include <array>
#include <vector>
#include <string>

namespace artik {

class DescriptorWrapper : public node::ObjectWrap {
 public:
  ~DescriptorWrapper();

  static void Init(v8::Local<v8::Object> exports);
  static bool HasInstance(v8::Local<v8::Value> obj);

  void Emit(int argc, v8::Local<v8::Value> argv[]);
  const artik_bt_gatt_desc& GetDesc() const { return m_descriptor; }
  void SetId(int desc_id);
  int GetDescriptorId() const;

 private:
  explicit DescriptorWrapper(const artik_bt_gatt_desc& descriptor);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static v8::Persistent<v8::FunctionTemplate> functionTemplate;
  static v8::Persistent<v8::Function> constructor;

  static int ConvertStringProperties(
    const std::vector<std::string>& strv, std::string *error);

  artik_bt_gatt_desc m_descriptor;
  int m_descriptor_id;
  v8::Persistent<v8::Function>* m_emit;
};

class CharacteristicWrapper : public node::ObjectWrap {
 public:
  ~CharacteristicWrapper();

  static void Init(v8::Local<v8::Object> exports);
  static bool HasInstance(v8::Local<v8::Value> obj);

  void Emit(int argc, v8::Local<v8::Value> argv[]);
  const artik_bt_gatt_chr& GetChar() const { return m_characteristic; }
  const std::vector<DescriptorWrapper*>& GetDescriptors() const {
    return m_descriptors;
  }
  void SetIds(int service_id, int char_id);
  int GetServiceId() const;
  int GetCharacteristicId() const;

 private:
  CharacteristicWrapper(
    const artik_bt_gatt_chr& m_characteristic,
    const std::vector<DescriptorWrapper*>& descriptors);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static v8::Persistent<v8::FunctionTemplate> functionTemplate;
  static v8::Persistent<v8::Function> constructor;

  static int ConvertStringProperties(
    const std::vector<std::string> &strv, std::string *error);

  artik_bt_gatt_chr m_characteristic;
  std::vector<DescriptorWrapper*> m_descriptors;
  v8::Persistent<v8::Function>* m_emit;
  int m_service_id;
  int m_characteristic_id;
};

class ServiceWrapper : public node::ObjectWrap {
 public:
  ~ServiceWrapper();

  static void Init(v8::Local<v8::Object> exports);
 private:
  ServiceWrapper(
    const artik_bt_gatt_service& service,
    const std::vector<CharacteristicWrapper*>& characteristics);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static v8::Persistent<v8::Function> constructor;

  Bluetooth* m_bt;
  int m_service_id;
  std::vector<CharacteristicWrapper*> m_characteristics;

  using CopyablePersistent =
    v8::CopyablePersistentTraits<v8::Object>::CopyablePersistent;
  std::vector<CopyablePersistent> m_persistent_objects;
};

class GattServerWrapper : public node::ObjectWrap {
 public:
  ~GattServerWrapper();

  static void Init(v8::Local<v8::Object> exports);

  Bluetooth* getObj() { return m_bt; }

 private:
  GattServerWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static v8::Persistent<v8::Function> constructor;

  static void start_advertising(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void stop_advertising(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void request_send_value(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void request_send_result(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void notify(const v8::FunctionCallbackInfo<v8::Value>& args);

  Bluetooth* m_bt;
  GlibLoop *m_loop;
};

}  // namespace artik

#endif  // ADDON_BLUETOOTH_GATT_SERVER_H_


