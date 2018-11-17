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

#include "bluetooth/gatt_client.h"

#include <artik_log.h>

#include <nan.h>
#include <utils.h>

namespace artik {

using v8::External;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::PropertyCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Object;
using v8::ObjectTemplate;
using v8::Persistent;
using v8::String;
using v8::Value;
using v8::Array;
using v8::Handle;
using v8::Int32;
using v8::Boolean;
using v8::Context;

Persistent<Function> GattClientWrapper::constructor;
Persistent<Function> RemoteServiceWrapper::constructor;
Persistent<Function> RemoteCharacteristicWrapper::constructor;
Persistent<Function> RemoteDescriptorWrapper::constructor;
std::vector<RemoteCharacteristicWrapper*>
    RemoteCharacteristicWrapper::s_characteristics;

RemoteDescriptorWrapper::RemoteDescriptorWrapper(const std::string& addr,
    const std::string& svc_uuid, const std::string& char_uuid,
    const std::string& desc_uuid)
  : m_addr(addr),
    m_srv_uuid(svc_uuid),
    m_char_uuid(char_uuid),
    m_desc_uuid(desc_uuid),
    m_bt(new Bluetooth()) {
}

RemoteDescriptorWrapper::~RemoteDescriptorWrapper() {
  delete m_bt;
}

void RemoteDescriptorWrapper::Init(v8::Local<v8::Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "RemoteDescriptor"));

  Local<v8::ObjectTemplate> objTpl = tpl->InstanceTemplate();
  objTpl->SetInternalFieldCount(1);

  objTpl->SetAccessor(Nan::New<String>("uuid").ToLocalChecked(), get_uuid);
  NODE_SET_PROTOTYPE_METHOD(tpl, "write", write);
  NODE_SET_PROTOTYPE_METHOD(tpl, "read", read);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "RemoteDescriptor"),
         tpl->GetFunction());
}

Local<Object> RemoteDescriptorWrapper::newInstance(const char *addr,
    const char *srv_uuid, const char* char_uuid, const char *desc_uuid ) {
  Isolate *isolate = Isolate::GetCurrent();

  Handle<Value> argv[] = {
    Nan::New<String>(addr).ToLocalChecked(),
    Nan::New<String>(srv_uuid).ToLocalChecked(),
    Nan::New<String>(char_uuid).ToLocalChecked(),
    Nan::New<String>(desc_uuid).ToLocalChecked()
  };

  Local<Context> context = isolate->GetCurrentContext();
  Local<Function> cons = Nan::New<Function>(constructor);
  return cons->NewInstance(context, 4, argv).ToLocalChecked();
}

void RemoteDescriptorWrapper::New(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  constexpr int expectedArgs = 4;
  Isolate *isolate = Isolate::GetCurrent();

  if (args.Length() != expectedArgs) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }
  if (args.IsConstructCall()) {
    auto addr = js_type_to_cpp<std::string>(args[0]);
    auto svc_uuid = js_type_to_cpp<std::string>(args[1]);
    auto char_uuid = js_type_to_cpp<std::string>(args[2]);
    auto desc_uuid = js_type_to_cpp<std::string>(args[3]);
    if (!addr || !svc_uuid || !char_uuid || !desc_uuid) {
      isolate->ThrowException(Nan::TypeError("Wrong argument type"));
      return;
    }
    RemoteDescriptorWrapper *obj = new RemoteDescriptorWrapper(addr.value(),
        svc_uuid.value(), char_uuid.value(), desc_uuid.value());
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Value> argv[expectedArgs];

    if (args.Length() == expectedArgs) {
      for (int i = 0; i < expectedArgs; ++i)
        argv[i] = args[i];
    }

    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, args.Length(), argv).ToLocalChecked());
  }
}

void RemoteDescriptorWrapper::write(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = Isolate::GetCurrent();
  RemoteDescriptorWrapper *obj =
    ObjectWrap::Unwrap<RemoteDescriptorWrapper>(args.Holder());
  Bluetooth* bt = obj->getObj();

  if (args.Length() != 1) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }

  if (!node::Buffer::HasInstance(args[0])) {
    isolate->ThrowException(Nan::TypeError("Wrong argument type"));
    return;
  }

  unsigned char *val = (unsigned char*) node::Buffer::Data(args[0]);
  size_t len = node::Buffer::Length(args[0]);

  artik_error err = bt->gatt_desc_write_value(obj->getAddr(), obj->getSrvUuid(),
        obj->getCharUuid(), obj->getDescUuid(), val, len);
  if (err != S_OK) {
    std::string error = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Nan::TypeError(error.c_str()));
    return;
  }
}

void RemoteDescriptorWrapper::read(
    const FunctionCallbackInfo<v8::Value>& args) {
  Isolate *isolate = Isolate::GetCurrent();
  RemoteDescriptorWrapper *obj =
    ObjectWrap::Unwrap<RemoteDescriptorWrapper>(args.Holder());
  Bluetooth* bt = obj->getObj();

  if (args.Length() != 0) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }

  unsigned char *bytes;
  int len;
  artik_error err = bt->gatt_desc_read_value(obj->getAddr(), obj->getSrvUuid(),
          obj->getCharUuid(), obj->getDescUuid(), &bytes, &len);
  if (err != S_OK) {
    std::string error = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Nan::TypeError(error.c_str()));
    return;
  }

  Local<Object> buffer = Nan::CopyBuffer(
      (const char*)bytes, len).ToLocalChecked();
  args.GetReturnValue().Set(buffer);

  free(bytes);
}

void RemoteDescriptorWrapper::get_uuid(Local<String> name,
    const PropertyCallbackInfo<Value> &info) {
  RemoteDescriptorWrapper *wrap = ObjectWrap::Unwrap<RemoteDescriptorWrapper>(
      info.Holder());
  Local<String> uuid = Nan::New<String>(wrap->getDescUuid()).ToLocalChecked();
  info.GetReturnValue().Set(uuid);
}

RemoteCharacteristicWrapper::RemoteCharacteristicWrapper(const std::string addr,
    const std::string& svc_uuid, const std::string& char_uuid)
  : m_addr(addr),
    m_srv_uuid(svc_uuid),
    m_char_uuid(char_uuid),
    m_bt(new Bluetooth()) {
}

RemoteCharacteristicWrapper::~RemoteCharacteristicWrapper() {
  delete m_bt;
  delete m_emit;
}

void RemoteCharacteristicWrapper::emit(int argc, Local<Value> argv[]) {
  Local<Function> emit_f = Nan::New<Function>(*m_emit);
  emit_f->Call(this->handle(), argc, argv);
}

void RemoteCharacteristicWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "RemoteCharacteristic"));

  Local<ObjectTemplate> objTpl = tpl->InstanceTemplate();
  objTpl->SetInternalFieldCount(1);

  objTpl->SetAccessor(Nan::New<String>("uuid").ToLocalChecked(), get_uuid);
  objTpl->SetAccessor(Nan::New<String>("properties").ToLocalChecked(),
      get_properties);

  NODE_SET_PROTOTYPE_METHOD(tpl, "discover_descriptors", discover_descriptors);
  NODE_SET_PROTOTYPE_METHOD(tpl, "write", write);
  NODE_SET_PROTOTYPE_METHOD(tpl, "read", read);
  NODE_SET_PROTOTYPE_METHOD(tpl, "subscribe", subscribe);
  NODE_SET_PROTOTYPE_METHOD(tpl, "unsubscribe", unsubscribe);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "RemoteCharacteristic"),
      tpl->GetFunction());
}

Local<Object> RemoteCharacteristicWrapper::newInstance(const char *addr,
    const char *srv_uuid, const char* char_uuid) {
  Isolate *isolate = Isolate::GetCurrent();

  Handle<Value> argv[] = {
    Nan::New<String>(addr).ToLocalChecked(),
    Nan::New<String>(srv_uuid).ToLocalChecked(),
    Nan::New<String>(char_uuid).ToLocalChecked(),
  };

  Local<Context> context = isolate->GetCurrentContext();
  Local<Function> cons = Nan::New<Function>(constructor);
  return cons->NewInstance(context, 3, argv).ToLocalChecked();
}

RemoteCharacteristicWrapper* RemoteCharacteristicWrapper::getCharacteristic(
    const char* srv_uuid, const char* char_uuid) {
  auto it = std::find_if(
      s_characteristics.begin(),
      s_characteristics.end(),
          [&](const RemoteCharacteristicWrapper* wrap) {
            return strcmp(srv_uuid, wrap->getSrvUuid()) == 0
                && strcmp(char_uuid, wrap->getCharUuid()) == 0;
           });

  if (it == s_characteristics.end())
    return NULL;

  return *it;
}

void RemoteCharacteristicWrapper::addCharacteristic(
    RemoteCharacteristicWrapper* wrap) {
  s_characteristics.push_back(wrap);
}

void RemoteCharacteristicWrapper::removeCharacteristic(
    RemoteCharacteristicWrapper* wrap) {
  auto it = std::remove_if(
      s_characteristics.begin(),
      s_characteristics.end(),
      [&](const RemoteCharacteristicWrapper* c) {
        return strcmp(wrap->getSrvUuid(), c->getSrvUuid()) == 0
            && strcmp(wrap->getCharUuid(), c->getCharUuid()) == 0;
      });
  s_characteristics.erase(it);
}

void RemoteCharacteristicWrapper::New(
    const FunctionCallbackInfo<Value>& args) {
  constexpr int expectedArgs = 3;
  Isolate *isolate = Isolate::GetCurrent();

  if (args.Length() != expectedArgs) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }

  if (args.IsConstructCall()) {
    auto addr = js_type_to_cpp<std::string>(args[0]);
    auto svc_uuid = js_type_to_cpp<std::string>(args[1]);
    auto char_uuid = js_type_to_cpp<std::string>(args[2]);
    if (!addr || !svc_uuid || !char_uuid) {
      isolate->ThrowException(Nan::TypeError("Wrong argument type"));
      return;
    }

    RemoteCharacteristicWrapper *obj = new RemoteCharacteristicWrapper(
        addr.value(), svc_uuid.value(), char_uuid.value());
    obj->Wrap(args.This());
    Local<Function> emit = Local<Function>::Cast(
        obj->handle()->Get(Nan::New<String>("emit").ToLocalChecked()));
    obj->m_emit = new v8::Persistent<v8::Function>(isolate, emit);
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Value> argv[expectedArgs];

    if (args.Length() == expectedArgs) {
      for (int i = 0; i < expectedArgs; ++i)
        argv[i] = args[i];
    }

    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, args.Length(), argv).ToLocalChecked());
  }
}

void RemoteCharacteristicWrapper::discover_descriptors(
    const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = Isolate::GetCurrent();
  RemoteCharacteristicWrapper* obj =
      ObjectWrap::Unwrap<RemoteCharacteristicWrapper>(args.Holder());
  Bluetooth* bt = obj->getObj();
  if (args.Length() != 0) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }

  artik_bt_uuid *uuid_list;
  int len;
  artik_error err = bt->gatt_get_descriptor_list(obj->getAddr(),
      obj->getSrvUuid(), obj->getCharUuid(), &uuid_list, &len);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Nan::TypeError(msg.c_str()));
    return;
  }

  Local<Array> array = Nan::New<v8::Array>();
  for (int i = 0; i < len; i++) {
    Local<Object> desc = RemoteDescriptorWrapper::newInstance(obj->getAddr(),
        obj->getSrvUuid(), obj->getCharUuid(), uuid_list[i].uuid);
    array->Set(i, desc);
    free(uuid_list[i].uuid);
    free(uuid_list[i].uuid_name);
  }
  free(uuid_list);

  args.GetReturnValue().Set(array);
}

void RemoteCharacteristicWrapper::write(
    const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = Isolate::GetCurrent();
  RemoteCharacteristicWrapper *obj =
    ObjectWrap::Unwrap<RemoteCharacteristicWrapper>(args.Holder());
  Bluetooth* bt = obj->getObj();

  if (args.Length() != 1) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }

  if (!node::Buffer::HasInstance(args[0])) {
    isolate->ThrowException(Nan::TypeError("Wrong argument type"));
    return;
  }

  unsigned char *val = (unsigned char*) node::Buffer::Data(args[0]);
  size_t len = node::Buffer::Length(args[0]);

  artik_error err = bt->gatt_char_write_value(obj->getAddr(), obj->getSrvUuid(),
      obj->getCharUuid(), val, len);
  if (err != S_OK) {
    std::string error = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Nan::TypeError(error.c_str()));
    return;
  }
}

void RemoteCharacteristicWrapper::read(
    const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = Isolate::GetCurrent();
  RemoteCharacteristicWrapper *obj =
    ObjectWrap::Unwrap<RemoteCharacteristicWrapper>(args.Holder());
  Bluetooth* bt = obj->getObj();

  if (args.Length() != 0) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }

  unsigned char *bytes;
  int len;
  artik_error err = bt->gatt_char_read_value(obj->getAddr(), obj->getSrvUuid(),
          obj->getCharUuid(), &bytes, &len);
  if (err != S_OK) {
    std::string error = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Nan::TypeError(error.c_str()));
    return;
  }

  Local<Object> buffer = Nan::CopyBuffer(
      (const char*)bytes, len).ToLocalChecked();
  args.GetReturnValue().Set(buffer);

  free(bytes);
}

void RemoteCharacteristicWrapper::subscribe(
    const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = Isolate::GetCurrent();
  RemoteCharacteristicWrapper *obj =
     ObjectWrap::Unwrap<RemoteCharacteristicWrapper>(args.Holder());
  Bluetooth* bt = obj->getObj();

  if (args.Length() != 0) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }

  RemoteCharacteristicWrapper::addCharacteristic(obj);
  artik_error err = bt->gatt_start_notify(obj->getAddr(), obj->getSrvUuid(),
      obj->getCharUuid());
  if (err != S_OK) {
    std::string error = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Nan::TypeError(error.c_str()));
    return;
  }
}

void RemoteCharacteristicWrapper::unsubscribe(
    const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = Isolate::GetCurrent();
  RemoteCharacteristicWrapper *obj =
    ObjectWrap::Unwrap<RemoteCharacteristicWrapper>(args.Holder());
  Bluetooth* bt = obj->getObj();

  if (args.Length() != 0) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }

  RemoteCharacteristicWrapper::removeCharacteristic(obj);
  artik_error err = bt->gatt_stop_notify(obj->getAddr(), obj->getSrvUuid(),
      obj->getCharUuid());
  if (err != S_OK) {
    std::string error = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Nan::TypeError(error.c_str()));
    return;
  }
}

void RemoteCharacteristicWrapper::get_uuid(Local<String> name,
    const PropertyCallbackInfo<Value> &info) {
  RemoteCharacteristicWrapper *wrap =
      ObjectWrap::Unwrap<RemoteCharacteristicWrapper>(info.Holder());

  Local<String> uuid = Nan::New<String>(wrap->getCharUuid()).ToLocalChecked();

  info.GetReturnValue().Set(uuid);
}

void  RemoteCharacteristicWrapper::get_properties(Local<v8::String> name,
    const PropertyCallbackInfo<Value>& info) {
  RemoteCharacteristicWrapper *wrap =
      ObjectWrap::Unwrap<RemoteCharacteristicWrapper>(info.Holder());

  Local<Array> properties = Nan::New<Array>();

  artik_bt_gatt_char_properties prop;
  artik_error err = wrap->getObj()->gatt_get_char_properties(wrap->getAddr(),
        wrap->getSrvUuid(), wrap->getCharUuid(), &prop);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    info.GetIsolate()->ThrowException(Nan::TypeError(msg.c_str()));
    return;
  }

  int i = 0;
  if (prop & BT_GATT_CHAR_PROPERTY_BROADCAST) {
    properties->Set(i, Nan::New<String>("broadcast").ToLocalChecked());
    i++;
  }

  if (prop & BT_GATT_CHAR_PROPERTY_READ) {
    properties->Set(i, Nan::New<String>("read").ToLocalChecked());
    i++;
  }

  if (prop & BT_GATT_CHAR_PROPERTY_WRITE_NO_RESPONSE) {
    properties->Set(i,
        Nan::New<String>("write-without-response").ToLocalChecked());
    i++;
  }

  if (prop & BT_GATT_CHAR_PROPERTY_WRITE) {
    properties->Set(i, Nan::New<String>("write").ToLocalChecked());
    i++;
  }

  if (prop & BT_GATT_CHAR_PROPERTY_NOTIFY) {
    properties->Set(i, Nan::New<String>("notify").ToLocalChecked());
    i++;
  }

  if (prop & BT_GATT_CHAR_PROPERTY_INDICATE) {
    properties->Set(i, Nan::New<String>("indicate").ToLocalChecked());
    i++;
  }

  if (prop & BT_GATT_CHAR_PROPERTY_SIGNED_WRITE) {
    properties->Set(i, Nan::New<String>("signed-write").ToLocalChecked());
    i++;
  }

  info.GetReturnValue().Set(properties);
}

RemoteServiceWrapper::RemoteServiceWrapper(const std::string& addr,
    const std::string& uuid)
  : m_addr(addr),
    m_srv_uuid(uuid),
    m_bt(new Bluetooth()) {
}

RemoteServiceWrapper::~RemoteServiceWrapper() {
  delete m_bt;
}

void RemoteServiceWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "RemoteService"));

  Local<v8::ObjectTemplate> objTpl = tpl->InstanceTemplate();
  objTpl->SetInternalFieldCount(1);

  objTpl->SetAccessor(Nan::New<String>("uuid").ToLocalChecked(), get_uuid);
  NODE_SET_PROTOTYPE_METHOD(tpl, "discover_characteristics",
      discover_characteristics);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "RemoteService"),
         tpl->GetFunction());
}

Local<Object> RemoteServiceWrapper::newInstance(const char *addr,
    const char *srv_uuid) {
  Isolate *isolate = Isolate::GetCurrent();

  Handle<Value> argv[] = {
    Nan::New<String>(addr).ToLocalChecked(),
    Nan::New<String>(srv_uuid).ToLocalChecked(),
  };

  Local<Context> context = isolate->GetCurrentContext();
  Local<Function> cons = Nan::New<Function>(constructor);
  return cons->NewInstance(context, 2, argv).ToLocalChecked();
}

void RemoteServiceWrapper::New(const FunctionCallbackInfo<Value>& args) {
  constexpr int expectedArgs = 2;
  Isolate *isolate = Isolate::GetCurrent();

  if (args.Length() != expectedArgs) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }
  if (args.IsConstructCall()) {
    auto addr = js_type_to_cpp<std::string>(args[0]);
    auto svc_uuid = js_type_to_cpp<std::string>(args[1]);
    if (!svc_uuid) {
      isolate->ThrowException(Nan::TypeError("Wrong argument type"));
      return;
    }

    RemoteServiceWrapper *obj = new RemoteServiceWrapper(addr.value(),
        svc_uuid.value());
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Value> argv[expectedArgs];

    if (args.Length() == expectedArgs) {
      for (int i = 0; i < expectedArgs; ++i)
        argv[i] = args[i];
    }

    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, args.Length(), argv).ToLocalChecked());
  }
}

void RemoteServiceWrapper::discover_characteristics(
    const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = Isolate::GetCurrent();
  RemoteServiceWrapper* obj = ObjectWrap::Unwrap<RemoteServiceWrapper>(
      args.Holder());
  Bluetooth* bt = obj->getObj();
  if (args.Length() != 0) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }

  artik_bt_uuid *uuid_list;
  int len;
  log_dbg("get characteristic of %s %s", obj->getAddr(), obj->getSrvUuid());
  artik_error err = bt->gatt_get_characteristic_list(obj->getAddr(),
      obj->getSrvUuid(), &uuid_list, &len);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Nan::TypeError(msg.c_str()));
    return;
  }

  Local<Array> array = Nan::New<v8::Array>();
  log_dbg("C list to JS Array");
  for (int i = 0; i < len; i++) {
    Local<Object> chr = RemoteCharacteristicWrapper::newInstance(obj->getAddr(),
          obj->getSrvUuid(), uuid_list[i].uuid);
    array->Set(i, chr);
    free(uuid_list[i].uuid);
    free(uuid_list[i].uuid_name);
  }
  free(uuid_list);

  args.GetReturnValue().Set(array);
}

void RemoteServiceWrapper::get_uuid(Local<String> name,
    const PropertyCallbackInfo<Value> &info) {
  RemoteServiceWrapper *wrap = ObjectWrap::Unwrap<RemoteServiceWrapper>(
      info.Holder());

  Local<String> uuid = Nan::New<String>(wrap->getSrvUuid()).ToLocalChecked();
  info.GetReturnValue().Set(uuid);
}

static void on_received(artik_bt_event event, void *data, void *user_data) {
  artik_bt_gatt_data *d = reinterpret_cast<artik_bt_gatt_data*>(data);
  Nan::HandleScope scope;
  RemoteCharacteristicWrapper* characteristic =
      RemoteCharacteristicWrapper::getCharacteristic(d->srv_uuid, d->char_uuid);

  if (characteristic == NULL)
    return;

  Local<Object> buffer = Nan::CopyBuffer((
        const char*)d->bytes, d->length).ToLocalChecked();
  Local<Value> argv[] = {
    Nan::New<String>("data").ToLocalChecked(),
    buffer
  };

  characteristic->emit(2, argv);
}

static void on_services_discover(artik_bt_event event, void *data,
    void *user_data) {
  GattClientWrapper *wrap = reinterpret_cast<GattClientWrapper*>(user_data);
  Nan::HandleScope scope;

  Local<Value> argv[] = {
    Nan::New<String>("servicesDiscover").ToLocalChecked()
  };

  wrap->emit(1, argv);
}

GattClientWrapper::GattClientWrapper()
  : m_bt(new Bluetooth()) {
  m_bt->set_callback(BT_EVENT_GATT_CHARACTERISTIC, on_received, NULL);
  m_bt->set_callback(BT_EVENT_SERVICE_RESOLVED, on_services_discover, this);
  m_loop = GlibLoop::Instance();
  m_loop->attach();
}

GattClientWrapper::~GattClientWrapper() {
  delete m_bt;
  delete m_emit;
  m_loop->detach();
}

void GattClientWrapper::emit(int argc, v8::Local<v8::Value> argv[]) {
  Local<Function> emit_f = Nan::New<Function>(*m_emit);
  emit_f->Call(this->handle(), argc, argv);
}

void GattClientWrapper::Init(v8::Local<v8::Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "GattClient"));

  Local<v8::ObjectTemplate> objTpl = tpl->InstanceTemplate();
  objTpl->SetInternalFieldCount(1);

  NODE_SET_PROTOTYPE_METHOD(tpl, "discover_services", discover_services);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "GattClient"),
         tpl->GetFunction());
}

void GattClientWrapper::New(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate *isolate = Isolate::GetCurrent();

  if (args.Length() != 0) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }
  if (args.IsConstructCall()) {
    GattClientWrapper *obj = new GattClientWrapper();
    obj->Wrap(args.This());
    Local<Function> emit = Local<Function>::Cast(obj->handle()->Get
        (Nan::New<String>("emit").ToLocalChecked()));
    obj->m_emit = new v8::Persistent<v8::Function>(isolate, emit);
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

void GattClientWrapper::discover_services(
    const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = Isolate::GetCurrent();
  Bluetooth* bt = ObjectWrap::Unwrap<GattClientWrapper>(
      args.Holder())->getObj();
  if (args.Length() != 1) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }

  auto addr = js_type_to_cpp<std::string>(args[0]);
  if (!addr) {
    isolate->ThrowException(Nan::TypeError("Wrong argument type"));
    return;
  }

  artik_bt_uuid *uuid_list;
  int len;
  artik_error err = bt->gatt_get_service_list(addr.value().c_str(),
      &uuid_list, &len);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Nan::TypeError(msg.c_str()));
    return;
  }

  Local<Array> array = Nan::New<Array>();
  for (int i = 0; i < len; i++) {
    Local<Object> srv = RemoteServiceWrapper::newInstance(addr.value().c_str(),
          uuid_list[i].uuid);
    array->Set(i, srv);
    free(uuid_list[i].uuid);
    free(uuid_list[i].uuid_name);
  }
  free(uuid_list);

  args.GetReturnValue().Set(array);
}

}  // namespace artik
