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

#include "bluetooth/spp.h"

#include <artik_log.h>
#include <artik_loop.h>

#include <node_buffer.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <utility>

#define MAX_PACKET_SIZE 1024

namespace artik {

using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::PropertyCallbackInfo;
using v8::Handle;
using v8::HandleScope;
using v8::Int32;
using v8::Isolate;
using v8::Local;
using v8::MaybeLocal;
using v8::Object;
using v8::ObjectTemplate;
using v8::Persistent;
using v8::String;
using v8::Value;
using v8::Null;
using v8::Context;

Persistent<Function> SppSocketWrapper::constructor;
Persistent<Function> SppWrapper::constructor;

static int _socket_watch(int fd, enum watch_io io, void *user_data) {
  int ret = 0;
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  SppSocketWrapper *wrapSocket = reinterpret_cast<SppSocketWrapper*>(user_data);
  Local<Value> error = Null(isolate);
  Local<Value> js_buffer = Null(isolate);

  if (io & WATCH_IO_IN) {
    uint8_t buffer[MAX_PACKET_SIZE];
    int num_bytes = 0;
    num_bytes = recv(fd, buffer, MAX_PACKET_SIZE, 0);
    if (num_bytes < 0) {
      std::string msg = "Error: " + std::string(strerror(errno));
      error = String::NewFromUtf8(isolate, msg.c_str());
    } else {
      MaybeLocal<Object> buf = node::Buffer::Copy(isolate,
          reinterpret_cast<char*>(buffer), num_bytes);
      js_buffer = buf.ToLocalChecked();
    }
    ret = 1;
  } else if (io & WATCH_IO_ERR) {
    std::string msg = "Error: unable to read the socket.";
    error = String::NewFromUtf8(isolate, msg.c_str());
  } else if (io & WATCH_IO_HUP) {
    std::string msg = "Error: The connection was broken.";
    error = String::NewFromUtf8(isolate, msg.c_str());
  } else if (io & WATCH_IO_NVAL) {
    std::string msg = "Error: Invalid request. The file descriptor is not open";
    error = String::NewFromUtf8(isolate, msg.c_str());
  }

  Handle<Value> argv[] = {
    String::NewFromUtf8(isolate, "data"),
    error,
    js_buffer
  };

  wrapSocket->emit(isolate, wrapSocket, 3, argv);
  return ret;
}

SppSocketWrapper::SppSocketWrapper(const std::string& device_path, int fd,
    int version, int features)
  : m_device_path(device_path),
    m_fd(fd),
    m_version(version),
    m_features(features) {
}

SppSocketWrapper::~SppSocketWrapper() {
  delete m_emit;
}

void SppSocketWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "SppSocket"));

  Local<ObjectTemplate> objTpl = tpl->InstanceTemplate();
  objTpl->SetInternalFieldCount(1);

  objTpl->SetAccessor(String::NewFromUtf8(isolate, "version"),
          getVersion);
  objTpl->SetAccessor(String::NewFromUtf8(isolate, "features"),
          getFeatures);
  NODE_SET_PROTOTYPE_METHOD(tpl, "write", write);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "SppSocket"),
         tpl->GetFunction());
}

void SppSocketWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  log_dbg("");

  if (args.IsConstructCall()) {
    if (args.Length() != 4) {
      isolate->ThrowException(Exception::TypeError(
              String::NewFromUtf8(isolate, "Wrong arguments")));
      return;
    }

    if (!args[0]->IsString() || !args[1]->IsInt32()
        || !args[2]->IsInt32() || !args[3]->IsInt32()) {
      isolate->ThrowException(Exception::TypeError(
              String::NewFromUtf8(isolate, "Wrong type arguments")));
      return;
    }

    String::Utf8Value device_path(args[0]->ToString());
    int fd = args[1]->Int32Value();
    int version = args[2]->Int32Value();
    int features = args[3]->Int32Value();
    artik_loop_module* loop =
        reinterpret_cast<artik_loop_module*>(artik_request_api_module("loop"));

    SppSocketWrapper* obj = new SppSocketWrapper(*device_path, fd, version,
        features);

    artik_error err = loop->add_fd_watch(
        fd,
        (watch_io)(WATCH_IO_IN | WATCH_IO_ERR | WATCH_IO_HUP | WATCH_IO_NVAL),
        _socket_watch,
        obj,
        NULL);

    if (err != S_OK) {
      delete obj;
      std::string msg = "Error: " + std::string(error_msg(err));
      isolate->ThrowException(Exception::Error(
            String::NewFromUtf8(isolate, msg.c_str())));
      return;
    }

    obj->Wrap(args.This());
    Local<Function> emit = Local<Function>::Cast(obj->handle()->Get(
        String::NewFromUtf8(isolate, "emit")));
    obj->m_emit = new Persistent<Function>(isolate, emit);

    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

void SppSocketWrapper::emit(Isolate *isolate, ObjectWrap* obj, int argc,
    Local<Value> argv[]) {
  Local<Function> emit_f = Local<Function>::New(isolate, *m_emit);
  node::MakeCallback(isolate, obj->handle(), emit_f, argc, argv);
}

Persistent<Object>* SppSocketWrapper::newInstance(Isolate *isolate,
    const std::string& device_path, int fd, int version, int features) {
  HandleScope scope(isolate);
  Handle<Value> argv[] = {
    String::NewFromUtf8(isolate, device_path.c_str()),
    Int32::New(isolate, fd),
    Int32::New(isolate, version),
    Int32::New(isolate, features)
  };

  Local<Context> context = isolate->GetCurrentContext();
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  return new Persistent<Object>(isolate,
      cons->NewInstance(context, 4, argv).ToLocalChecked());
}

void SppSocketWrapper::getVersion(Local<String> property,
    const PropertyCallbackInfo<Value>& info) {
  Isolate *isolate = info.GetIsolate();
  SppSocketWrapper* sppSocket = ObjectWrap::Unwrap<SppSocketWrapper>(
      info.Holder());

  info.GetReturnValue().Set(Int32::New(isolate, sppSocket->m_version));
}

void SppSocketWrapper::getFeatures(Local<String> property,
    const PropertyCallbackInfo<Value>& info) {
  Isolate *isolate = info.GetIsolate();

  SppSocketWrapper* sppSocket = ObjectWrap::Unwrap<SppSocketWrapper>(
      info.Holder());

  info.GetReturnValue().Set(Int32::New(isolate, sppSocket->m_features));
}

void SppSocketWrapper::write(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  SppSocketWrapper* obj = ObjectWrap::Unwrap<SppSocketWrapper>(args.Holder());

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  if (!node::Buffer::HasInstance(args[0])) {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Wrong type arguments")));
  }

  char * buffer = node::Buffer::Data(args[0]);
  size_t len = node::Buffer::Length(args[0]);

  ssize_t written_bytes = ::write(obj->m_fd, buffer, len);
  if (written_bytes < 0) {
    std::string msg = "Error: " + std::string(strerror(errno));
    isolate->ThrowException(Exception::Error(
          String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

static void _spp_release(artik_bt_event event,
    void *data, void *user_data) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  SppWrapper *wrap = reinterpret_cast<SppWrapper*>(user_data);

  Handle<Value> argv[] = {
    String::NewFromUtf8(isolate, "release"),
  };

  wrap->emit(isolate, wrap, 1, argv);
}

static void _spp_new_connection(artik_bt_event event,
    void *data, void *user_data) {
  artik_bt_spp_connect_property *spp_property =
      reinterpret_cast<artik_bt_spp_connect_property *>(data);

  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  SppWrapper *wrap = reinterpret_cast<SppWrapper*>(user_data);

  Persistent<Object>* spp_socket = SppSocketWrapper::newInstance(isolate,
      spp_property->device_addr,
      spp_property->fd,
      spp_property->version,
      spp_property->features);
  Handle<Value> argv[] = {
    String::NewFromUtf8(isolate, "new_connection"),
    Local<Object>::New(isolate, *spp_socket)
  };

  wrap->addSocket(std::string(spp_property->device_addr), spp_socket);
  wrap->emit(isolate, wrap, 2, argv);
}

static void _spp_request_disconnect(artik_bt_event event,
    void *data, void *user_data) {
  char *device_path = reinterpret_cast<char *>(data);

  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  SppWrapper *wrap = reinterpret_cast<SppWrapper*>(user_data);

  Local<Object> js_sppSocket = Local<Object>::Cast(
      wrap->removeSocket(device_path));
  SppSocketWrapper* wrapSocket =
      node::ObjectWrap::Unwrap<SppSocketWrapper>(js_sppSocket);

  Handle<Value> argv[] = {
    String::NewFromUtf8(isolate, "disconnect"),
    js_sppSocket
  };

  wrapSocket->emit(isolate, wrap, 2, argv);
}

static bool _js_object_to_artik_bt_spp_profile_option(
    artik_bt_spp_profile_option *opt, Local<Object> obj) {
  Isolate *isolate = Isolate::GetCurrent();
  Handle<Value> name = obj->Get(String::NewFromUtf8(isolate, "name"));
  Handle<Value> service = obj->Get(String::NewFromUtf8(isolate, "service"));
  Handle<Value> role = obj->Get(String::NewFromUtf8(isolate, "role"));
  Handle<Value> channel = obj->Get(String::NewFromUtf8(isolate, "channel"));
  Handle<Value> PSM = obj->Get(String::NewFromUtf8(isolate, "PSM"));
  Handle<Value> require_authentication = obj->Get(
      String::NewFromUtf8(isolate, "require_authentication"));
  Handle<Value> require_authorization = obj->Get(
      String::NewFromUtf8(isolate, "require_authorization"));
  Handle<Value> auto_connect = obj->Get(
      String::NewFromUtf8(isolate, "auto_connect"));
  Handle<Value> version = obj->Get(String::NewFromUtf8(isolate, "version"));
  Handle<Value> features = obj->Get(String::NewFromUtf8(isolate, "features"));

  if (!name->IsString()) {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Wrong type arguments")));
    return false;
  }

  if (!service->IsString()) {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Wrong type arguments")));
    return false;
  }

  if (!role->IsString()) {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Wrong type arguments")));
    return false;
  }

  if (!channel->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Wrong type arguments")));
    return false;
  }

  if (!PSM->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Wrong type arguments")));
    return false;
  }

  if (!require_authentication->IsBoolean()) {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Wrong type arguments")));
    return false;
  }

  if (!require_authorization->IsBoolean()) {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Wrong type arguments")));
    return false;
  }

  if (!auto_connect->IsBoolean()) {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Wrong type arguments")));
    return false;
  }

  if (!version->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Wrong type arguments")));
    return false;
  }

  if (!features->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, "Wrong type arguments")));
    return false;
  }

  String::Utf8Value utf8_value(name->ToString());
  String::Utf8Value utf8_service(service->ToString());
  String::Utf8Value utf8_role(role->ToString());

  opt->name = strdup(*utf8_value);
  opt->service = strdup(*utf8_service);
  opt->role = strdup(*utf8_role);
  opt->channel = channel->Int32Value();
  opt->PSM = PSM->Int32Value();
  opt->require_authentication = require_authentication->BooleanValue();
  opt->require_authorization = require_authorization->BooleanValue();
  opt->auto_connect = auto_connect->BooleanValue();
  opt->version = version->Int32Value();
  opt->features = features->Int32Value();

  return true;
}

SppWrapper::SppWrapper()
  : m_bt(new Bluetooth()) {
  m_loop = GlibLoop::Instance();
  m_loop->attach();
}

SppWrapper::~SppWrapper() {
  delete m_emit;
  delete m_bt;

  for (auto& value : m_sockets) {
    delete value.second;
  }

  m_loop->detach();
}

void SppWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "Spp"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "register_profile", spp_register_profile);
  NODE_SET_PROTOTYPE_METHOD(tpl, "unregister_profile", spp_unregister_profile);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "Spp"),
         tpl->GetFunction());
}

void SppWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  log_dbg("");

  if (args.IsConstructCall()) {
    SppWrapper* obj = new SppWrapper();
    obj->Wrap(args.This());
    Local<Function> emit =
      Local<Function>::Cast(obj->handle()->Get(
          String::NewFromUtf8(isolate, "emit")));
    obj->m_emit = new Persistent<Function>(isolate, emit);
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

void SppWrapper::emit(Isolate* isolate, ObjectWrap* obj, int argc,
    Local<Value> argv[]) {
  Local<Function> emit_f = Local<Function>::New(isolate, *m_emit);
  node::MakeCallback(isolate, obj->handle(), emit_f, argc, argv);
}

void SppWrapper::addSocket(const std::string& device_path,
    Persistent<Object>* js_socket) {
  m_sockets.insert(std::pair<std::string,
      Persistent<Object>*>(device_path, js_socket));
}

Local<Value> SppWrapper::removeSocket(const std::string& device_path) {
  Isolate *isolate = Isolate::GetCurrent();
  auto it = m_sockets.find(device_path);

  if (it == m_sockets.cend()) {
    return Null(isolate);
  }

  Local<Object> js_socket = Local<Object>::New(isolate, *(it->second));
  delete it->second;

  m_sockets.erase(it);

  return js_socket;
}

void SppWrapper::spp_register_profile(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  SppWrapper *obj = ObjectWrap::Unwrap<SppWrapper>(args.Holder());
  Bluetooth* bt = obj->getObj();

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsObject()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong type arguments")));
  }

  artik_bt_spp_profile_option opts;
  if (!_js_object_to_artik_bt_spp_profile_option(&opts, args[0]->ToObject()))
    return;

  artik_error err = bt->spp_register_profile(&opts);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
          String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }

  artik_bt_callback_property spp_callbacks[] = {
      {BT_EVENT_SPP_CONNECT, _spp_new_connection,
          reinterpret_cast<void *>(obj)},
      {BT_EVENT_SPP_RELEASE, _spp_release, reinterpret_cast<void *>(obj)},
      {BT_EVENT_SPP_DISCONNECT, _spp_request_disconnect,
          reinterpret_cast<void *>(obj)}
  };

  err = bt->set_callbacks(spp_callbacks, 3);

  if (err != S_OK) {
    bt->spp_unregister_profile();
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
          String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void SppWrapper::spp_unregister_profile(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<SppWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  artik_error err = obj->spp_unregister_profile();
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

}  // namespace artik

