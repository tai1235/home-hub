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

#include "websocket/websocket.h"

#include <unistd.h>
#include <artik_log.h>

#include <memory>
#include <string>

#include "base/ssl_config_converter.h"

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
using v8::HandleScope;
using v8::Int32;
using v8::Context;

Persistent<Function> WebsocketWrapper::constructor;

static void websocket_connection_callback(void* user_data, void* result) {
  Isolate * isolate = Isolate::GetCurrent();
  HandleScope handleScope(isolate);
  WebsocketWrapper* wrap = reinterpret_cast<WebsocketWrapper*>(user_data);
  uint32_t ret = reinterpret_cast<uintptr_t>(result);

  log_dbg("");

  if (!wrap->getConnectionCb())
    return;

  if (ret == ARTIK_WEBSOCKET_CLOSED) {
    Handle<Value> argv[] = {
      Handle<Value>(String::NewFromUtf8(isolate, "CLOSED")),
    };
    Local<Function>::New(isolate, *wrap->getConnectionCb())->Call(
        isolate->GetCurrentContext()->Global(), 1, argv);
    return;
  } else if (ret == ARTIK_WEBSOCKET_CONNECTED) {
    Handle<Value> argv[] = {
      Handle<Value>(String::NewFromUtf8(isolate, "CONNECTED")),
    };
    Local<Function>::New(isolate, *wrap->getConnectionCb())->Call(
        isolate->GetCurrentContext()->Global(), 1, argv);
    return;
  } else if (ret == ARTIK_WEBSOCKET_HANDSHAKE_ERROR) {
    Handle<Value> argv[] = {
      Handle<Value>(String::NewFromUtf8(isolate, "HANDSHAKE ERROR")),
    };
    Local<Function>::New(isolate, *wrap->getConnectionCb())->Call(
        isolate->GetCurrentContext()->Global(), 1, argv);
    return;
  }

  log_err("Wrong value for callback result");
}

static void websocket_receive_callback(void* user_data, void* message) {
  Isolate * isolate = Isolate::GetCurrent();
  HandleScope handleScope(isolate);
  WebsocketWrapper* wrap = reinterpret_cast<WebsocketWrapper*>(user_data);

  log_dbg("");

  if (!wrap->getReceiveCb()) {
    log_err("wrong arg");
    return;
  }

  Handle<Value> argv[] = {
    Handle<Value>(String::NewFromUtf8(isolate,
          reinterpret_cast<char*>(message)))
  };
  free(message);
  Local<Function>::New(isolate, *wrap->getReceiveCb())->Call(
      isolate->GetCurrentContext()->Global(), 1, argv);
}

WebsocketWrapper::WebsocketWrapper() {
  m_websocket = NULL;
  m_loop = GlibLoop::Instance();
  m_loop->attach();
}

WebsocketWrapper::WebsocketWrapper(char* uri, artik_ssl_config *ssl_config) {
  m_websocket = new Websocket(uri, ssl_config);
  m_loop = GlibLoop::Instance();
  m_loop->attach();
}

WebsocketWrapper::~WebsocketWrapper() {
  // clean up routine
  if (m_websocket)
    delete m_websocket;
  m_loop->detach();
}

void WebsocketWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "websocket"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

    // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "open_stream", open_stream);
  NODE_SET_PROTOTYPE_METHOD(tpl, "write_stream", write_stream);
  NODE_SET_PROTOTYPE_METHOD(tpl, "close_stream", close_stream);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "websocket"), tpl->GetFunction());
}

void WebsocketWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  int lenArg = 2;
  std::unique_ptr<artik_ssl_config> ssl_config;

  if (args.Length() != lenArg) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  } else if (args.IsConstructCall()) {
    WebsocketWrapper* obj = NULL;
    char* uri = NULL;

    if (args[0]->IsString()) {
      String::Utf8Value param0(args[0]->ToString());
      uri = strndup(*param0, strlen(*param0));
    } else {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
      return;
    }

    if (args[1]->IsObject())
      ssl_config = SSLConfigConverter::convert(isolate, args[1]);

    obj = new WebsocketWrapper(uri, ssl_config.get());
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

void WebsocketWrapper::open_stream(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  WebsocketWrapper* wrap = ObjectWrap::Unwrap<WebsocketWrapper>(args.Holder());
  Websocket* obj = wrap->getObj();

  if (obj->request() != S_OK) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Failed to request a Handle")));
    return;
  }

  if (obj->open_stream() != S_OK) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Failed to open websocket stream")));
    return;
  }

  // If callback is provided, run a work for waiting on connection complete
  if (args[0]->IsFunction()) {
    wrap->m_connection_cb = new Persistent<Function>();
    wrap->m_connection_cb->Reset(isolate, Local<Function>::Cast(args[0]));
    obj->set_connection_callback(websocket_connection_callback,
        reinterpret_cast<void *>(wrap));
  }
  if (args[1]->IsFunction()) {
    wrap->m_receive_cb = new Persistent<Function>();
    wrap->m_receive_cb->Reset(isolate, Local<Function>::Cast(args[1]));
    obj->set_receive_callback(websocket_receive_callback,
        reinterpret_cast<void*>(wrap));
  }

  args.GetReturnValue().Set(Number::New(isolate, S_OK));
}

void WebsocketWrapper::write_stream(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  WebsocketWrapper* obj = ObjectWrap::Unwrap<WebsocketWrapper>(args.Holder());
  Websocket* websocket = obj->getObj();

  String::Utf8Value param0(args[0]->ToString());
  char* message = *param0;

  artik_error ret = websocket->write_stream(message);
  if (ret != S_OK) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Failed to write a Message")));
    return;
  }

  args.GetReturnValue().Set(Number::New(isolate, ret));
}

void WebsocketWrapper::close_stream(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  WebsocketWrapper* obj = ObjectWrap::Unwrap<WebsocketWrapper>(args.Holder());
  Websocket* websocket = obj->getObj();

  args.GetReturnValue().Set(Number::New(isolate, websocket->close_stream()));
}

}  // namespace artik

