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

#include "media/media.h"

#include <unistd.h>

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
using v8::Context;

Persistent<Function> MediaWrapper::constructor;

static void on_finished(void *userdata) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  MediaWrapper *wrap = reinterpret_cast<MediaWrapper*>(userdata);

  if (!wrap->getFinishedCb())
    return;

  Local<Function>::New(isolate, *wrap->getFinishedCb())->Call(
      isolate->GetCurrentContext()->Global(), 0, NULL);
}

MediaWrapper::MediaWrapper() {
  m_media = new Media();
  m_loop = GlibLoop::Instance();
  m_loop->attach();
}

MediaWrapper::~MediaWrapper() {
  delete m_media;
  m_loop->detach();
}

void MediaWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "media"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "play_sound_file", play_sound_file);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "media"), tpl->GetFunction());
}

void MediaWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.IsConstructCall()) {
    MediaWrapper* obj = new MediaWrapper();
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

void MediaWrapper::play_sound_file(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  MediaWrapper* wrap = ObjectWrap::Unwrap<MediaWrapper>(args.Holder());
  Media* obj = wrap->getObj();
  artik_error ret = S_OK;

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
  }

  v8::String::Utf8Value param0(args[0]->ToString());

  ret = obj->play_sound_file(*param0);
  if (ret != S_OK) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Failed to play sound")));
  }

  // Register finsihed callback if passed as an argument
  if (args[1]->IsFunction()) {
    wrap->m_finished_cb = new v8::Persistent<v8::Function>();
    wrap->m_finished_cb->Reset(isolate, Local<Function>::Cast(args[1]));
    obj->set_finished_callback(on_finished, reinterpret_cast<void*>(wrap));
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

}  // namespace artik
