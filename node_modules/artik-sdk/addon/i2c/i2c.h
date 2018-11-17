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

#ifndef ADDON_I2C_I2C_H_
#define ADDON_I2C_I2C_H_

#include <node.h>
#include <node_object_wrap.h>

#include <artik_i2c.hh>

namespace artik {

class I2cWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  I2c* getObj() { return m_i2c; }

 private:
  explicit I2cWrapper(artik_i2c_id id, int frequency,
                      artik_i2c_wordsize_t wordsize, unsigned int address);
  ~I2cWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void request(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void release(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void read(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void write(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void read_register(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void write_register(const v8::FunctionCallbackInfo<v8::Value>& args);

  I2c* m_i2c;
};

}  // namespace artik

#endif  // ADDON_I2C_I2C_H_
