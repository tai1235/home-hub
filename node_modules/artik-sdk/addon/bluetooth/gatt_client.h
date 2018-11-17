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

#ifndef ADDON_BLUETOOTH_GATT_CLIENT_H_
#define ADDON_BLUETOOTH_GATT_CLIENT_H_

#include <node.h>
#include <node_object_wrap.h>

#include <artik_bluetooth.hh>
#include <loop.h>

#include <vector>
#include <string>

namespace artik {

class RemoteDescriptorWrapper : public node::ObjectWrap {
 public:
  ~RemoteDescriptorWrapper();

  const char* getAddr() const { return m_addr.c_str(); }
  const char* getSrvUuid() const { return m_srv_uuid.c_str(); }
  const char* getCharUuid() const { return m_char_uuid.c_str(); }
  const char* getDescUuid() const { return m_desc_uuid.c_str(); }
  Bluetooth* getObj() { return m_bt; }

  static void Init(v8::Local<v8::Object> exports);
  static v8::Local<v8::Object> newInstance(const char *addr,
      const char *srv_uuid, const char *char_uuid, const char *desc_uuid);

 private:
  RemoteDescriptorWrapper(const std::string& addr, const std::string& svc_uuid,
    const std::string& char_uuid, const std::string& desc_uuid);
  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void write(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void read(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_uuid(v8::Local<v8::String> name,
      const v8::PropertyCallbackInfo<v8::Value> &info);

  std::string m_addr;
  std::string m_srv_uuid;
  std::string m_char_uuid;
  std::string m_desc_uuid;
  Bluetooth *m_bt;
};

class RemoteCharacteristicWrapper : public node::ObjectWrap {
 public:
  ~RemoteCharacteristicWrapper();

  const char* getAddr() const { return m_addr.c_str(); }
  const char* getSrvUuid() const { return m_srv_uuid.c_str(); }
  const char* getCharUuid() const { return m_char_uuid.c_str(); }
  Bluetooth* getObj() { return m_bt; }
  void emit(int argc, v8::Local<v8::Value> argv[]);

  static void Init(v8::Local<v8::Object> exports);
  static v8::Local<v8::Object> newInstance(const char *addr,
      const char *srv_uuid, const char* char_uuid);
  static RemoteCharacteristicWrapper* getCharacteristic(const char* srv_uuid,
      const char* char_uuid);
  static void addCharacteristic(RemoteCharacteristicWrapper* wrap);
  static void removeCharacteristic(RemoteCharacteristicWrapper* wrap);

 private:
  RemoteCharacteristicWrapper(const std::string addr,
      const std::string& svc_uuid, const std::string& char_uuid);
  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void discover_descriptors(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void write(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void read(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void subscribe(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void unsubscribe(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_uuid(v8::Local<v8::String> name,
      const v8::PropertyCallbackInfo<v8::Value> &info);
  static void get_properties(v8::Local<v8::String> name,
      const v8::PropertyCallbackInfo<v8::Value>& info);

  std::string m_addr;
  std::string m_srv_uuid;
  std::string m_char_uuid;
  Bluetooth *m_bt;
  v8::Persistent<v8::Function>* m_emit;

  static std::vector<RemoteCharacteristicWrapper*> s_characteristics;
};

class RemoteServiceWrapper : public node::ObjectWrap {
 public:
  ~RemoteServiceWrapper();

  const char* getAddr() const { return m_addr.c_str(); }
  const char* getSrvUuid() const { return m_srv_uuid.c_str(); }
  Bluetooth* getObj() { return m_bt; }

  static void Init(v8::Local<v8::Object> exports);
  static v8::Local<v8::Object> newInstance(const char *addr,
      const char *srv_uuid);

 private:
  RemoteServiceWrapper(const std::string& addr, const std::string& uuid);
  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void discover_characteristics(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_uuid(v8::Local<v8::String> name,
      const v8::PropertyCallbackInfo<v8::Value> &info);

  std::string m_addr;
  std::string m_srv_uuid;
  Bluetooth *m_bt;
};

class GattClientWrapper : public node::ObjectWrap {
 public:
  ~GattClientWrapper();

  void emit(int argc, v8::Local<v8::Value> argv[]);
  Bluetooth* getObj() { return m_bt; }

  static void Init(v8::Local<v8::Object> exports);

 private:
  GattClientWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void discover_services(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  Bluetooth *m_bt;
  v8::Persistent<v8::Function>* m_emit;
  GlibLoop *m_loop;
};

}  // namespace artik

#endif  // ADDON_BLUETOOTH_GATT_CLIENT_H_
