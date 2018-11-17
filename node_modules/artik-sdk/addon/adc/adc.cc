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

#include "adc/adc.h"

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

Persistent<Function> AdcWrapper::constructor;

AdcWrapper::AdcWrapper(unsigned int pin_num, char *name) {
  m_adc = new Adc(pin_num, name);
}

AdcWrapper::~AdcWrapper() {
  delete m_adc;
}

void AdcWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();
  Local<FunctionTemplate> modal = FunctionTemplate::New(isolate, New);
  modal->SetClassName(String::NewFromUtf8(isolate, "adc"));
  modal->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(modal, "request", request);
  NODE_SET_PROTOTYPE_METHOD(modal, "release", release);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_value", get_value);

  NODE_SET_PROTOTYPE_METHOD(modal, "get_pin_num", get_pin_num);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_name", get_name);

  NODE_SET_PROTOTYPE_METHOD(modal, "set_pin_num", set_pin_num);
  NODE_SET_PROTOTYPE_METHOD(modal, "set_name", set_name);

  constructor.Reset(isolate, modal->GetFunction());
  exports->Set(v8::String::NewFromUtf8(isolate, "adc"),
    modal->GetFunction());
}


void AdcWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  int lenArg = 2;
  if (args.Length() != lenArg && args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
                      isolate, "Wrong number of arguments")));
    return;
  }

  if (args.IsConstructCall()) {
    AdcWrapper* obj = NULL;

    if (args[0]->IsUint32() && args[1]->IsString()) {
      obj = new AdcWrapper(args[0]->Uint32Value(),
                           *v8::String::Utf8Value(args[1]->ToString()));
    } else if (args.Length() == 0) {
      obj = new AdcWrapper();
    } else {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
                isolate, "Wrong number of arguments or arguments type")));
      return;
    }

    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Value> argv[lenArg];

    if (args.Length() == lenArg) {
      for (int i = 0; i < lenArg; ++i)
        argv[i] = args[i];
    }

    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, args.Length(), argv).ToLocalChecked());
  }
}

void AdcWrapper::request(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
                      isolate, "Wrong number of arguments")));
    return;
  }

  Adc* obj = ObjectWrap::Unwrap<AdcWrapper>(args.Holder())->getObj();

  args.GetReturnValue().Set(Number::New(isolate, obj->request()));
}

void AdcWrapper::release(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
                      isolate, "Wrong number of arguments")));
    return;
  }
  Adc* obj = ObjectWrap::Unwrap<AdcWrapper>(args.Holder())->getObj();

  args.GetReturnValue().Set(Number::New(isolate, obj->release()));
}

void AdcWrapper::get_value(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
                        isolate, "Wrong number of arguments")));
    return;
  }

  Adc* obj = ObjectWrap::Unwrap<AdcWrapper>(args.Holder())->getObj();
  int val = -1;
  obj->get_value(&val);

  args.GetReturnValue().Set(Number::New(isolate, val));
}

void AdcWrapper::get_pin_num(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
                      isolate, "Wrong number of arguments")));
    return;
  }
  Adc* obj = ObjectWrap::Unwrap<AdcWrapper>(args.Holder())->getObj();

  args.GetReturnValue().Set(Number::New(isolate,
      static_cast<int>(obj->get_pin_num())));
}

void AdcWrapper::get_name(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
                      isolate, "Wrong number of arguments")));
    return;
  }
  Adc* obj = ObjectWrap::Unwrap<AdcWrapper>(args.Holder())->getObj();

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, obj->get_name()));
}

void AdcWrapper::set_pin_num(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
                      isolate, "Wrong type arguments")));
    return;
  }
  Adc* obj = ObjectWrap::Unwrap<AdcWrapper>(args.Holder())->getObj();

  obj->set_pin_num(static_cast<int>(args[0]->NumberValue()));
}

void AdcWrapper::set_name(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
                      isolate, "Wrong type arguments")));
    return;
  }

  Adc* obj = ObjectWrap::Unwrap<AdcWrapper>(args.Holder())->getObj();
  v8::String::Utf8Value val(args[0]->ToString());

  obj->set_name(*val);
}

}  // namespace artik
