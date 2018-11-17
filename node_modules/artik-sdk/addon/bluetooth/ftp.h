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

#ifndef ADDON_BLUETOOTH_FTP_H_
#define ADDON_BLUETOOTH_FTP_H_

#include <node.h>
#include <node_object_wrap.h>

#include <artik_bluetooth.hh>
#include <loop.h>

#include <array>

namespace artik {

class FtpWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  Bluetooth* getObj() { return m_bt; }

  void emit(v8::Isolate *isolate, node::ObjectWrap *obj, int argc,
      v8::Local<v8::Value> argv[]);

 private:
  FtpWrapper();
  ~FtpWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static v8::Persistent<v8::Function> constructor;

  static void create_session(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void remove_session(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void change_folder(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void create_folder(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void delete_file(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void list_folder(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_file(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void put_file(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void resume_transfer(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void suspend_transfer(const v8::FunctionCallbackInfo<v8::Value>& args);

  Bluetooth *m_bt;
  v8::Persistent<v8::Function>* m_emit;
  GlibLoop *m_loop;
};

}  // namespace artik

#endif  // ADDON_BLUETOOTH_FTP_H_

