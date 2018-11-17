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

#ifndef ADDON_SPI_SPI_H_
#define ADDON_SPI_SPI_H_

#include <node.h>
#include <node_object_wrap.h>

#include <artik_spi.hh>

namespace artik {

class SpiWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  Spi* getObj() { return m_spi; }

 private:
  SpiWrapper(unsigned int bus, unsigned int cs, artik_spi_mode mode,
      unsigned int bits_per_word, unsigned int speed);
  ~SpiWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void request(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void release(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void read(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void write(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void read_write(const v8::FunctionCallbackInfo<v8::Value>& args);

  Spi* m_spi;
};

}  // namespace artik

#endif  // ADDON_SPI_SPI_H_

