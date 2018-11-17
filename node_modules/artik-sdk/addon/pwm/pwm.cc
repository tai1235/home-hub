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

#include "pwm/pwm.h"

namespace artik {

using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;
using v8::Context;

Persistent<Function> PwmWrapper::constructor;

PwmWrapper::PwmWrapper(unsigned int pin_num, char *name, unsigned int period,
    artik_pwm_polarity_t  polarity, unsigned int duty_cycle) {
  m_pwm = new Pwm(pin_num, name, period, polarity, duty_cycle);
}

PwmWrapper::~PwmWrapper() {
  delete m_pwm;
}

void PwmWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();
  Local<FunctionTemplate> modal = FunctionTemplate::New(isolate, New);

  modal->SetClassName(String::NewFromUtf8(isolate, "pwm"));
  modal->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(modal, "request", request);
  NODE_SET_PROTOTYPE_METHOD(modal, "release", release);
  NODE_SET_PROTOTYPE_METHOD(modal, "enable", enable);
  NODE_SET_PROTOTYPE_METHOD(modal, "disable", disable);
  NODE_SET_PROTOTYPE_METHOD(modal, "set_period", set_period);
  NODE_SET_PROTOTYPE_METHOD(modal, "set_polarity", set_polarity);
  NODE_SET_PROTOTYPE_METHOD(modal, "set_duty_cycle", set_duty_cycle);

  NODE_SET_PROTOTYPE_METHOD(modal, "get_pin_num", get_pin_num);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_name", get_name);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_period", get_period);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_polarity", get_polarity);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_duty_cycle", get_duty_cycle);

  NODE_SET_PROTOTYPE_METHOD(modal, "set_pin_num", set_pin_num);
  NODE_SET_PROTOTYPE_METHOD(modal, "set_name", set_name);

  constructor.Reset(isolate, modal->GetFunction());
  exports->Set(v8::String::NewFromUtf8(isolate, "pwm"),
      modal->GetFunction());
}

void PwmWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  int lenArg = 5;
  if (args.Length() != lenArg && args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  if (args.IsConstructCall()) {
    PwmWrapper* obj = NULL;

    if (args.Length() == lenArg) {
      obj = new PwmWrapper(args[0]->Uint32Value(),
          *v8::String::Utf8Value(args[1]->ToString()),
          (unsigned int)args[2]->Int32Value(),
          (artik_pwm_polarity_t)args[3]->Int32Value(),
          (unsigned int)args[4]->Int32Value());
    } else {
      obj = new PwmWrapper();
    }

    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    int argc = args.Length();
    Local<Value> argv[argc];

    for (int i = 0; i < argc; ++i)
        argv[i] = args[i];

    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, argc, argv).ToLocalChecked());
  }
}

void PwmWrapper::request(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  Pwm* obj = ObjectWrap::Unwrap<PwmWrapper>(args.Holder())->getObj();
  args.GetReturnValue().Set(Number::New(isolate, obj->request()));
}

void PwmWrapper::release(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  Pwm* obj = ObjectWrap::Unwrap<PwmWrapper>(args.Holder())->getObj();
  args.GetReturnValue().Set(Number::New(isolate, obj->release()));
}

void PwmWrapper::enable(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  Pwm* obj = ObjectWrap::Unwrap<PwmWrapper>(args.Holder())->getObj();
  args.GetReturnValue().Set(Number::New(isolate, obj->enable()));
}

void PwmWrapper::disable(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  Pwm* obj = ObjectWrap::Unwrap<PwmWrapper>(args.Holder())->getObj();
  args.GetReturnValue().Set(Number::New(isolate, obj->disable()));
}

void PwmWrapper::set_period(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 1)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  Pwm* obj = ObjectWrap::Unwrap<PwmWrapper>(args.Holder())->getObj();
  args.GetReturnValue().Set(Number::New(isolate,
      obj->set_period(static_cast<unsigned int>(args[0]->NumberValue()))));
}

void PwmWrapper::set_polarity(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 1)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  Pwm* obj = ObjectWrap::Unwrap<PwmWrapper>(args.Holder())->getObj();
  args.GetReturnValue().Set(Number::New(isolate,
      obj->set_polarity((artik_pwm_polarity_t)args[0]->NumberValue())));
}

void PwmWrapper::set_duty_cycle(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 1)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  Pwm* obj = ObjectWrap::Unwrap<PwmWrapper>(args.Holder())->getObj();
  args.GetReturnValue().Set(Number::New(isolate,
      obj->set_duty_cycle(static_cast<unsigned int>(args[0]->NumberValue()))));
}

void PwmWrapper::get_pin_num(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  Pwm* obj = ObjectWrap::Unwrap<PwmWrapper>(args.Holder())->getObj();
  args.GetReturnValue().Set(
      Number::New(isolate, static_cast<int>(obj->get_pin_num())));
}

void PwmWrapper::get_name(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  Pwm* obj = ObjectWrap::Unwrap<PwmWrapper>(args.Holder())->getObj();
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, obj->get_name()));
}

void PwmWrapper::get_period(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  Pwm* obj = ObjectWrap::Unwrap<PwmWrapper>(args.Holder())->getObj();
  args.GetReturnValue().Set(
      Number::New(isolate, static_cast<int>(obj->get_period())));
}

void PwmWrapper::get_polarity(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  Pwm* obj = ObjectWrap::Unwrap<PwmWrapper>(args.Holder())->getObj();
  args.GetReturnValue().Set(Number::New(isolate, obj->get_polarity()));
}

void PwmWrapper::get_duty_cycle(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  Pwm* obj = ObjectWrap::Unwrap<PwmWrapper>(args.Holder())->getObj();
  args.GetReturnValue().Set(Number::New(isolate, obj->get_duty_cycle()));
}

void PwmWrapper::set_pin_num(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 1)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  Pwm* obj = ObjectWrap::Unwrap<PwmWrapper>(args.Holder())->getObj();
  obj->set_pin_num(static_cast<int>(args[0]->NumberValue()));
}

void PwmWrapper::set_name(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 1)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  Pwm* obj = ObjectWrap::Unwrap<PwmWrapper>(args.Holder())->getObj();
  v8::String::Utf8Value val(args[0]->ToString());

  obj->set_name(*val);
}

}  // namespace artik

