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

#include "bluetooth/pan.h"

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

Persistent<Function> PanWrapper::constructor;

PanWrapper::PanWrapper()
  : m_bt(new Bluetooth()) {
  m_loop = GlibLoop::Instance();
  m_loop->attach();
}

PanWrapper::~PanWrapper() {
  delete m_bt;
  m_loop->detach();
}

void PanWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "Pan"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "register", pan_register);
  NODE_SET_PROTOTYPE_METHOD(tpl, "unregister", pan_unregister);
  NODE_SET_PROTOTYPE_METHOD(tpl, "connect", pan_connect);
  NODE_SET_PROTOTYPE_METHOD(tpl, "disconnect", pan_disconnect);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_connected", pan_is_connected);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_interface", pan_get_interface);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_UUID", pan_get_UUID);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "Pan"),
         tpl->GetFunction());
}

void PanWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  log_dbg("");

  if (args.IsConstructCall()) {
    PanWrapper* obj = new PanWrapper();
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

void PanWrapper::pan_register(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<PanWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 2) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsString() || !args[1]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments type")));
    return;
  }

  String::Utf8Value val(args[0]->ToString());
  String::Utf8Value val1(args[1]->ToString());
  artik_error err = obj->pan_register(*val, *val1);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void PanWrapper::pan_unregister(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<PanWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments type")));
    return;
  }

  String::Utf8Value val(args[0]->ToString());
  artik_error err = obj->pan_unregister(*val);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void PanWrapper::pan_connect(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<PanWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 2) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsString() || !args[1]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments type")));
    return;
  }

  String::Utf8Value val(args[0]->ToString());
  String::Utf8Value val1(args[1]->ToString());
  char *network_interface = NULL;

  artik_error err = obj->pan_connect(*val, *val1, &network_interface);

  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, network_interface));
  g_free(network_interface);
}

void PanWrapper::pan_disconnect(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<PanWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  artik_error err = obj->pan_disconnect();
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void PanWrapper::pan_is_connected(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<PanWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  bool connected = obj->pan_is_connected();

  args.GetReturnValue().Set(Boolean::New(isolate, connected));
}

void PanWrapper::pan_get_interface(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<PanWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  char *interface;
  artik_error err = obj->pan_get_interface(&interface);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, interface));
}

void PanWrapper::pan_get_UUID(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<PanWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  char *uuid;
  artik_error err = obj->pan_get_UUID(&uuid);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, uuid));
}

}  // namespace artik

