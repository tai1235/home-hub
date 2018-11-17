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

#ifndef ADDON_ADC_ADC_H_
#define ADDON_ADC_ADC_H_

#include <node.h>
#include <nan.h>
#include <node_object_wrap.h>

#include <uv.h>
#include <artik_adc.hh>

namespace artik {

class AdcWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object>);

  Adc* getObj() { return m_adc; }

 private:
  AdcWrapper(unsigned int, char*);
  AdcWrapper();
  ~AdcWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void request(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void release(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_value(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void get_pin_num(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_name(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void set_pin_num(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void set_name(const v8::FunctionCallbackInfo<v8::Value>& args);

  Adc *m_adc;
};

}  // namespace artik


#endif  // ADDON_ADC_ADC_H_
