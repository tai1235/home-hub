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

#ifndef ADDON_BLUETOOTH_A2DP_H_
#define ADDON_BLUETOOTH_A2DP_H_

#include <node.h>
#include <node_object_wrap.h>

#include <artik_bluetooth.hh>
#include <loop.h>

namespace artik {

class A2dpWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  Bluetooth* getObj() { return m_bt; }

 private:
  A2dpWrapper();
  ~A2dpWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static v8::Persistent<v8::Function> constructor;

  static void a2dp_source_get_state(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  Bluetooth *m_bt;
  GlibLoop *m_loop;
};

}  // namespace artik

#endif  // ADDON_BLUETOOTH_A2DP_H_

