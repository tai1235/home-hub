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

#include "bluetooth/a2dp.h"

#include <artik_log.h>
#include <glib.h>

#include <string>

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
using v8::Array;
using v8::Handle;
using v8::Int32;
using v8::Boolean;
using v8::Context;

Persistent<Function> A2dpWrapper::constructor;

A2dpWrapper::A2dpWrapper()
  : m_bt(new Bluetooth()) {
  m_loop = GlibLoop::Instance();
  m_loop->attach();
}

A2dpWrapper::~A2dpWrapper() {
  delete m_bt;
  m_loop->detach();
}

void A2dpWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "A2dp"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "source_get_state", a2dp_source_get_state);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "A2dp"),
         tpl->GetFunction());
}

void A2dpWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  log_dbg("");

  if (args.IsConstructCall()) {
    A2dpWrapper* obj = new A2dpWrapper();
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

void A2dpWrapper::a2dp_source_get_state(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<A2dpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  char *state;
  artik_error err = obj->a2dp_source_get_state(&state);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }

  Local<String> js_state = String::NewFromUtf8(isolate, state);
  args.GetReturnValue().Set(js_state);
  g_free(state);
}

}  // namespace artik

