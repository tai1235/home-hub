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

#ifndef ADDON_SERIAL_SERIAL_H_
#define ADDON_SERIAL_SERIAL_H_

#include <node.h>
#include <nan.h>
#include <node_object_wrap.h>

#include <uv.h>
#include <artik_serial.hh>
#include <loop.h>

#include <array>

namespace artik {

class SerialWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  Serial* getObj() { return m_serial; }
  int GetRxBufSize() { return m_rx_buf_size; }
  v8::Persistent<v8::Function>* getChangeCb() { return m_change_cb; }

 private:
  SerialWrapper(unsigned int, char*, artik_serial_baudrate_t,
      artik_serial_parity_t, artik_serial_data_bits_t,
      artik_serial_stop_bits_t, artik_serial_flowcontrol_t);
  SerialWrapper();
  ~SerialWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void request(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void release(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void write(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void get_port_num(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_name(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_baudrate(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_parity(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_data_bits(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_stop_bits(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_flowctrl(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void set_port_num(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void set_name(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void set_baudrate(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void set_parity(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void set_data_bits(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void set_stop_bits(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void set_flowctrl(const v8::FunctionCallbackInfo<v8::Value>& args);

  static const std::array<int, 7> s_baudrates;
  static const std::array<const char *, 3> s_parities;
  static const std::array<int, 2> s_data_bits;
  static const std::array<int, 2> s_stop_bits;
  static const std::array<const char*, 3> s_flowcontrols;

  Serial* m_serial;
  v8::Persistent<v8::Function>* m_change_cb;
  int m_rx_buf_size;
  GlibLoop* m_loop;
};

}  // namespace artik

#endif  // ADDON_SERIAL_SERIAL_H_

