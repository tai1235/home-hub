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

#include "i2c/i2c.h"

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

Persistent<Function> I2cWrapper::constructor;

I2cWrapper::I2cWrapper(artik_i2c_id id, int frequency,
    artik_i2c_wordsize_t wordsize, unsigned int address) {
  m_i2c = new I2c(id, frequency, wordsize, address);
}

I2cWrapper::~I2cWrapper() {
  delete m_i2c;
}

void I2cWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "i2c"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "request", request);
  NODE_SET_PROTOTYPE_METHOD(tpl, "release", release);
  NODE_SET_PROTOTYPE_METHOD(tpl, "read", read);
  NODE_SET_PROTOTYPE_METHOD(tpl, "write", write);
  NODE_SET_PROTOTYPE_METHOD(tpl, "read_register", read_register);
  NODE_SET_PROTOTYPE_METHOD(tpl, "write_register", write_register);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "i2c"),
               tpl->GetFunction());
}

void I2cWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.IsConstructCall()) {
    if (!args[0]->IsNumber() || !args[1]->IsNumber() || !args[2]->IsString()
        || !args[3]->IsNumber()) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Wrong arguments")));
      return;
    }

    artik_i2c_id id = args[0]->NumberValue();
    int frequency = args[1]->NumberValue();

    artik_i2c_wordsize_t wordsize = artik_i2c_wordsize_t::I2C_WORDSIZE_INVALID;
    v8::String::Utf8Value param2(args[2]->ToString());
    char* word_str = *param2;

    if (!strncmp(word_str, "8", MAX_ARG_STR_LEN)) {
      wordsize = artik_i2c_wordsize_t::I2C_8BIT;
    } else if (!strncmp(word_str, "16", MAX_ARG_STR_LEN)) {
      wordsize = artik_i2c_wordsize_t::I2C_16BIT;
    } else {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Wrong arguments")));
      return;
    }

    unsigned char address = args[3]->NumberValue();

    I2cWrapper* obj = new I2cWrapper(id, frequency, wordsize, address);
      obj->Wrap(args.This());
      args.GetReturnValue().Set(args.This());
  } else {
    const int argc = 4;
    Local<Value> argv[argc] = { args[0], args[1], args[2], args[3] };
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, argc, argv).ToLocalChecked());
  }
}

void I2cWrapper::request(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  I2c* obj = ObjectWrap::Unwrap<I2cWrapper>(args.Holder())->getObj();
  artik_error ret = obj->request();

  args.GetReturnValue().Set(Number::New(isolate, ret));
}

void I2cWrapper::release(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  I2c* obj = ObjectWrap::Unwrap<I2cWrapper>(args.Holder())->getObj();
  artik_error ret = obj->release();

  args.GetReturnValue().Set(Number::New(isolate, ret));
}

void I2cWrapper::read(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  I2c* obj = ObjectWrap::Unwrap<I2cWrapper>(args.Holder())->getObj();

  if (args.Length() != 1 || !args[0]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Wrong arguments")));
    return;
  }

  int length = args[0]->NumberValue();

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

void I2cWrapper::write(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 1) {
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
  size_t length = node::Buffer::Length(args[0]);

  I2c* obj = ObjectWrap::Unwrap<I2cWrapper>(args.Holder())->getObj();

  artik_error ret = obj->write(buffer, length);

  args.GetReturnValue().Set(Number::New(isolate, ret));
}

void I2cWrapper::read_register(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  I2c* obj = ObjectWrap::Unwrap<I2cWrapper>(args.Holder())->getObj();

  if (args.Length() != 2 || !args[0]->IsNumber() || !args[1]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Wrong arguments")));
    return;
  }

  unsigned int address = args[0]->NumberValue();
  int length = args[1]->NumberValue();

  char* buffer = reinterpret_cast<char*>(malloc(length));
  if (!buffer) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Memory allocation error")));
    return;
  }

  obj->read_register(address, buffer, length);

  args.GetReturnValue().Set(Nan::CopyBuffer(buffer, length).ToLocalChecked());

  free(buffer);
}

void I2cWrapper::write_register(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  I2c* obj = ObjectWrap::Unwrap<I2cWrapper>(args.Holder())->getObj();

  if (args.Length() != 2 || !args[0]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Wrong arguments")));
    return;
  }

  if (!node::Buffer::HasInstance(args[1])) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
                isolate, "Argument should be a Buffer.")));
      return;
  }

  unsigned int address = args[0]->NumberValue();
  char * buffer = node::Buffer::Data(args[1]);
  size_t length = node::Buffer::Length(args[1]);

  artik_error ret = obj->write_register(address, buffer, length);

  args.GetReturnValue().Set(Number::New(isolate, ret));
}

}  // namespace artik
