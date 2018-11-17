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

#include "spi/spi.h"

#include <unistd.h>
#include <node_buffer.h>
#include <nan.h>

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
using v8::Context;

Persistent<Function> SpiWrapper::constructor;

SpiWrapper::SpiWrapper(unsigned int bus, unsigned int cs, artik_spi_mode mode,
  unsigned int bits_per_word, unsigned int speed) {
  m_spi = new Spi(bus, cs, mode, bits_per_word, speed);
}

SpiWrapper::~SpiWrapper() {
  delete m_spi;
}

void SpiWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "spi"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "request", request);
  NODE_SET_PROTOTYPE_METHOD(tpl, "release", release);
  NODE_SET_PROTOTYPE_METHOD(tpl, "read", request);
  NODE_SET_PROTOTYPE_METHOD(tpl, "write", release);
  NODE_SET_PROTOTYPE_METHOD(tpl, "read_write", read_write);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "spi"),
               tpl->GetFunction());
}

void SpiWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.IsConstructCall()) {
    if ((args.Length() < 5) ||
        !args[0]->IsNumber() ||
        !args[1]->IsNumber() ||
        !args[2]->IsNumber() ||
        !args[3]->IsNumber() ||
        !args[4]->IsNumber()) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Wrong arguments")));
      return;
    }

    unsigned int bus = args[0]->NumberValue();
    unsigned int cs = args[1]->NumberValue();
    artik_spi_mode mode = (artik_spi_mode)(args[2]->NumberValue());
    unsigned int bits_per_word = args[3]->NumberValue();
    unsigned int speed = args[4]->NumberValue();

    SpiWrapper* obj = new SpiWrapper(bus, cs, mode, bits_per_word, speed);
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    const int argc = 5;
    Local<Value> argv[argc] = { args[0], args[1], args[2], args[3], args[4]};
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, argc, argv).ToLocalChecked());
  }
}

void SpiWrapper::request(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  Spi* obj = ObjectWrap::Unwrap<SpiWrapper>(args.Holder())->getObj();
  artik_error ret = obj->request();

  args.GetReturnValue().Set(Number::New(isolate, ret));
}

void SpiWrapper::release(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  Spi* obj = ObjectWrap::Unwrap<SpiWrapper>(args.Holder())->getObj();
  artik_error ret = obj->release();

  args.GetReturnValue().Set(Number::New(isolate, ret));
}

void SpiWrapper::read(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Spi* obj = ObjectWrap::Unwrap<SpiWrapper>(args.Holder())->getObj();

  if (args.Length() < 1 || !args[0]->IsUint32()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Wrong arguments")));
    return;
  }

  unsigned int length = args[0]->Uint32Value();
  char* buffer = reinterpret_cast<char*>(malloc(length));

  if (!buffer) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Memory allocation error")));
    return;
  }

  obj->read(buffer, length);

  args.GetReturnValue().Set(Nan::CopyBuffer(buffer, length).ToLocalChecked());

  free(buffer);
}

void SpiWrapper::write(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() < 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  if (!node::Buffer::HasInstance(args[0])) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Argument should be a Buffer.")));
    return;
  }

  char * buffer = node::Buffer::Data(args[0]);
  unsigned int length = node::Buffer::Length(args[0]);

  Spi* obj = ObjectWrap::Unwrap<SpiWrapper>(args.Holder())->getObj();

  artik_error ret = obj->write(buffer, length);

  args.GetReturnValue().Set(Number::New(isolate, ret));
}

void SpiWrapper::read_write(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Spi* obj = ObjectWrap::Unwrap<SpiWrapper>(args.Holder())->getObj();

  if (args.Length() < 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  if (!node::Buffer::HasInstance(args[0])) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Argument should be a Buffer.")));
    return;
  }

  char *tx_buffer = node::Buffer::Data(args[0]);
  unsigned int length = node::Buffer::Length(args[0]);
  char *rx_buffer = reinterpret_cast<char*>(malloc(length));

  if (!rx_buffer) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Memory allocation error")));
    return;
  }

  obj->read_write(tx_buffer, rx_buffer, length);

  args.GetReturnValue().Set(
      Nan::CopyBuffer(rx_buffer, length).ToLocalChecked());

  free(rx_buffer);
}

}  // namespace artik

