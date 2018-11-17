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

#ifndef ADDON_GPIO_GPIO_H_
#define ADDON_GPIO_GPIO_H_

#include <node.h>
#include <node_object_wrap.h>

#include <uv.h>
#include <artik_gpio.hh>

#include <loop.h>

namespace artik {

class GpioWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  Gpio* getObj() { return m_gpio; }
  v8::Persistent<v8::Function>* getChangeCb() { return m_change_cb; }

 private:
  explicit GpioWrapper(artik_gpio_id, char*, artik_gpio_dir_t,
      artik_gpio_edge_t, int);
  ~GpioWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void request(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void release(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void read(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void write(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_name(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_direction(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_id(const v8::FunctionCallbackInfo<v8::Value>& args);

  Gpio* m_gpio;
  v8::Persistent<v8::Function>* m_change_cb;
  GlibLoop* m_loop;
};

}  // namespace artik


#endif  // ADDON_GPIO_GPIO_H_
