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

#ifndef ADDON_BLUETOOTH_SPP_H_
#define ADDON_BLUETOOTH_SPP_H_

#include <artik_bluetooth.hh>

#include <node.h>
#include <node_object_wrap.h>
#include <loop.h>

#include <map>
#include <string>

namespace artik {

class SppSocketWrapper : public node::ObjectWrap {
 public:
  ~SppSocketWrapper();
  static void Init(v8::Local<v8::Object> exports);

  static v8::Persistent<v8::Object>* newInstance(
      v8::Isolate *isolate,
      const std::string& device_path,
      int fd,
      int version ,
      int features);

  void emit(v8::Isolate *isolate, node::ObjectWrap *obj, int argc,
      v8::Local<v8::Value> argv[]);

 private:
  SppSocketWrapper(const std::string& device_path, int fd, int version,
      int features);
  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void getVersion(v8::Local<v8::String> property,
      const v8::PropertyCallbackInfo<v8::Value>& info);
  static void getFeatures(v8::Local<v8::String> property,
      const v8::PropertyCallbackInfo<v8::Value>& info);
  static void write(const v8::FunctionCallbackInfo<v8::Value>& args);
  std::string m_device_path;
  int m_fd;
  int m_version;
  int m_features;

  v8::Persistent<v8::Function>* m_emit;
};

class SppWrapper : public node::ObjectWrap {
 public:
  ~SppWrapper();
  static void Init(v8::Local<v8::Object> exports);

  Bluetooth* getObj() { return m_bt; }

  void emit(v8::Isolate *isolate, node::ObjectWrap* obj, int argc,
      v8::Local<v8::Value> argv[]);

  void addSocket(
    const std::string& device_path,
    v8::Persistent<v8::Object>* js_socket);
  v8::Local<v8::Value> removeSocket(const std::string& device_path);

 private:
  SppWrapper();
  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void spp_register_profile(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void spp_unregister_profile(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  Bluetooth* m_bt;
  GlibLoop *m_loop;

  v8::Persistent<v8::Function>* m_emit;
  std::map<std::string, v8::Persistent<v8::Object>*> m_sockets;
};

}  // namespace artik

#endif  // ADDON_BLUETOOTH_SPP_H_

