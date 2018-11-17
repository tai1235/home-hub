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

#include "gpio/gpio.h"

#include <unistd.h>

#include <artik_log.h>

#define MAX_ARG_STR_LEN 32

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
using v8::Handle;
using v8::Context;

Persistent<Function> GpioWrapper::constructor;

static void gpio_change_callback(void* user_data, int val) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  GpioWrapper* wrap = reinterpret_cast<GpioWrapper*>(user_data);

  log_dbg("");

  if (!wrap->getChangeCb())
    return;

  Handle<Value> argv[] = {
    Handle<Value>(String::NewFromUtf8(isolate, val ? "1" : "0")),
  };

  Local<Function>::New(isolate, *wrap->getChangeCb())->Call(
      isolate->GetCurrentContext()->Global(), 1, argv);
}

GpioWrapper::GpioWrapper(artik_gpio_id id, char* name, artik_gpio_dir_t dir,
    artik_gpio_edge_t edge, int initial_value) {
  m_gpio = new Gpio(id, name, dir, edge, initial_value);
  m_change_cb = NULL;
  m_loop = GlibLoop::Instance();
  m_loop->attach();
}

GpioWrapper::~GpioWrapper() {
  m_loop->detach();
  delete m_gpio;
}

void GpioWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "gpio"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "request", request);
  NODE_SET_PROTOTYPE_METHOD(tpl, "release", release);
  NODE_SET_PROTOTYPE_METHOD(tpl, "read", read);
  NODE_SET_PROTOTYPE_METHOD(tpl, "write", write);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_name", get_name);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_direction", get_direction);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_id", get_id);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "gpio"),
               tpl->GetFunction());
}

void GpioWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  log_dbg("");

  if (args.IsConstructCall()) {
    if (!args[0]->IsNumber() ||
        !args[1]->IsString() ||
        !args[2]->IsString() ||
        !args[3]->IsString() ||
        !args[4]->IsNumber()) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Wrong arguments")));
        return;
    }

    artik_gpio_id id = args[0]->NumberValue();

    v8::String::Utf8Value param1(args[1]->ToString());
    char* name = *param1;

    artik_gpio_dir_t dir = artik_gpio_dir_t::GPIO_DIR_INVALID;
    v8::String::Utf8Value param2(args[2]->ToString());
    char* dir_str = *param2;

    if (!strncmp(dir_str, "out", MAX_ARG_STR_LEN)) {
      dir = GPIO_OUT;
    } else if (!strncmp(dir_str, "in", MAX_ARG_STR_LEN)) {
      dir = GPIO_IN;
    } else {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Wrong arguments")));
      return;
    }

    artik_gpio_edge_t edge = artik_gpio_edge_t::GPIO_EDGE_NONE;
    v8::String::Utf8Value param3(args[3]->ToString());
    char* edge_str = *param3;
    if (!strncmp(edge_str, "rising", MAX_ARG_STR_LEN))
        edge = GPIO_EDGE_RISING;
    else if (!strncmp(edge_str, "falling", MAX_ARG_STR_LEN))
        edge = GPIO_EDGE_FALLING;
    else if (!strncmp(edge_str, "both", MAX_ARG_STR_LEN))
        edge = GPIO_EDGE_BOTH;
    else
        edge = GPIO_EDGE_NONE;

    int initial_value = args[4]->NumberValue();

    GpioWrapper* obj = new GpioWrapper(id, name, dir, edge, initial_value);
    obj->Wrap(args.This());

    args.GetReturnValue().Set(args.This());
  } else {
    const int argc = 5;
    Local<Value> argv[argc] = { args[0], args[1], args[2], args[3], args[4] };
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, argc, argv).ToLocalChecked());
  }
}

void GpioWrapper::request(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  GpioWrapper* wrap = ObjectWrap::Unwrap<GpioWrapper>(args.Holder());
  Gpio* obj = wrap->getObj();
  artik_error ret;

  log_dbg("");

  ret = obj->request();

  /* If a callback is provided, use it for change notification */
  if ((ret == S_OK) && args[0]->IsFunction() &&
      (obj->get_direction() == GPIO_IN)) {
    wrap->m_change_cb = new v8::Persistent<v8::Function>();
    wrap->m_change_cb->Reset(isolate, Local<Function>::Cast(args[0]));
    obj->set_change_callback(gpio_change_callback,
        reinterpret_cast<void*>(wrap));
  }

  args.GetReturnValue().Set(Number::New(isolate, ret));
}

void GpioWrapper::release(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  GpioWrapper* wrap = ObjectWrap::Unwrap<GpioWrapper>(args.Holder());
  Gpio* obj = wrap->getObj();
  artik_error ret;

  log_dbg("");

  /* If a callback was set, release it */
  if (wrap->m_change_cb) {
    obj->unset_change_callback();
    delete wrap->m_change_cb;
    wrap->m_change_cb = NULL;
  }

  ret = obj->release();

  args.GetReturnValue().Set(Number::New(isolate, ret));
}

void GpioWrapper::read(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Gpio* obj = ObjectWrap::Unwrap<GpioWrapper>(args.Holder())->getObj();
  int val;

  log_dbg("");

  val = obj->read();

  if (val < 0) {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, error_msg(val))));
    return;
  }

  args.GetReturnValue().Set(Number::New(isolate, val));
}

void GpioWrapper::write(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  log_dbg("");

  if (args.Length() < 1 || !args[0]->IsNumber()) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
            isolate, "Wrong arguments")));
      return;
  }

  Gpio* obj = ObjectWrap::Unwrap<GpioWrapper>(args.Holder())->getObj();
  artik_error ret = obj->write(args[0]->NumberValue());

  args.GetReturnValue().Set(Number::New(isolate, ret));
}

void GpioWrapper::get_name(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Gpio* obj = ObjectWrap::Unwrap<GpioWrapper>(args.Holder())->getObj();

  log_dbg("");

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, obj->get_name()));
}

void GpioWrapper::get_direction(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Gpio* obj = ObjectWrap::Unwrap<GpioWrapper>(args.Holder())->getObj();

  log_dbg("");

  args.GetReturnValue().Set(String::NewFromUtf8(isolate,
      (obj->get_direction() == GPIO_OUT) ? "out" : "in"));
}

void GpioWrapper::get_id(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Gpio* obj = ObjectWrap::Unwrap<GpioWrapper>(args.Holder())->getObj();

  log_dbg("");

  args.GetReturnValue().Set(Number::New(isolate, obj->get_id()));
}

}  // namespace artik
