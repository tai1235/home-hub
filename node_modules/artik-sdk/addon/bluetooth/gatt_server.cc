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

#include "bluetooth/gatt_server.h"

#include <artik_log.h>

#include <nan.h>
#include <nan_converters.h>
#include <nan_new.h>
#include <node_buffer.h>

#include <utils.h>

namespace artik {

using v8::External;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
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

Persistent<Function> ServiceWrapper::constructor;
Persistent<Function> CharacteristicWrapper::constructor;
Persistent<Function> DescriptorWrapper::constructor;
Persistent<Function> GattServerWrapper::constructor;

Persistent<FunctionTemplate> DescriptorWrapper::functionTemplate;
Persistent<FunctionTemplate> CharacteristicWrapper::functionTemplate;

void DescriptorWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "Descriptor"));

  Local<ObjectTemplate> objTpl = tpl->InstanceTemplate();
  objTpl->SetInternalFieldCount(1);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "Descriptor"),
         tpl->GetFunction());

  functionTemplate.Reset(isolate, tpl);
}

bool DescriptorWrapper::HasInstance(Local<Value> obj) {
  Isolate *isolate = Isolate::GetCurrent();
  return Local<FunctionTemplate>::New(isolate,
      functionTemplate)->HasInstance(obj);
}

int DescriptorWrapper::ConvertStringProperties(
  const std::vector<std::string> &strv, std::string *error) {
  std::array<const char *, 8> properties = {
    "read",
    "write",
    "enc-read",
    "enc-write",
    "enc-auth-read",
    "enc-auth-write",
    "sec-read",
    "sec-write"
  };

  std::array<int, 8> c_properties = {
    BT_GATT_DESC_PROPERTY_READ,
    BT_GATT_DESC_PROPERTY_WRITE,
    BT_GATT_DESC_PROPERTY_ENC_READ,
    BT_GATT_DESC_PROPERTY_ENC_WRITE,
    BT_GATT_DESC_PROPERTY_ENC_AUTH_READ,
    BT_GATT_DESC_PROPERTY_ENC_AUTH_WRITE,
    BT_GATT_DESC_PROPERTY_SEC_READ,
    BT_GATT_DESC_PROPERTY_SEC_WRITE
  };

  int prop = 0;
  for (const auto& str : strv) {
    auto property = to_artik_parameter<artik_bt_gatt_desc_properties>(
        properties, str.c_str());
    if (!property) {
      *error = "Property " + str + " is not supported";
      return -1;
    }

    prop |= c_properties[property.value()];
  }

  return prop;
}

void DescriptorWrapper::New(const FunctionCallbackInfo<Value>& args) {
  constexpr int expectedArgs = 3;
  Isolate *isolate = args.GetIsolate();
  if (args.Length() != expectedArgs) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }

  if (args.IsConstructCall()) {
    artik_bt_gatt_desc descriptor;

    auto uuid = js_type_to_cpp<std::string>(args[0]);
    auto properties = js_type_to_cpp<std::vector<std::string>>(args[1]);

    if (!uuid || !properties) {
      isolate->ThrowException(Nan::TypeError("Wrong argument type"));
      return;
    }

    std::string error;
    descriptor.property = DescriptorWrapper::ConvertStringProperties(
      properties.value(), &error);
    if (descriptor.property == -1) {
      isolate->ThrowException(Nan::TypeError(error.c_str()));
      return;
    }

    descriptor.uuid = strdup(uuid.value().c_str());
    if (node::Buffer::HasInstance(args[2])) {
      unsigned char *val = (unsigned char*)(node::Buffer::Data(args[2]));
      size_t len = node::Buffer::Length(args[2]);
      descriptor.value = (unsigned char*)malloc(sizeof(unsigned char)*len);
      descriptor.length = len;
      memcpy(descriptor.value, val, len);
    } else {
      descriptor.value = NULL;
      descriptor.length = 0;
    }
    DescriptorWrapper* obj = new DescriptorWrapper(descriptor);
    obj->Wrap(args.This());
    Local<Function> emit = Local<Function>::Cast(obj->handle()->Get(
        String::NewFromUtf8(isolate, "emit")));
    obj->m_emit = new Persistent<Function>(isolate, emit);

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

DescriptorWrapper::DescriptorWrapper(const artik_bt_gatt_desc& descriptor)
  : m_descriptor(descriptor) {
}

DescriptorWrapper::~DescriptorWrapper() {
  free(m_descriptor.uuid);
  if (m_descriptor.value)
    free(m_descriptor.value);
  delete m_emit;
}

void DescriptorWrapper::Emit(int argc, Local<Value> argv[]) {
  Local<Function> emit_f = Nan::New<Function>(*m_emit);
  emit_f->Call(this->handle(), argc, argv);
}

void DescriptorWrapper::SetId(int desc_id) {
  m_descriptor_id = desc_id;
}

int DescriptorWrapper::GetDescriptorId() const {
  return m_descriptor_id;
}

bool CharacteristicWrapper::HasInstance(Local<Value> obj) {
  Isolate *isolate = Isolate::GetCurrent();
  return Local<FunctionTemplate>::New(isolate,
      functionTemplate)->HasInstance(obj);
}

void CharacteristicWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "Characteristic"));

  Local<ObjectTemplate> objTpl = tpl->InstanceTemplate();
  objTpl->SetInternalFieldCount(1);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "Characteristic"),
         tpl->GetFunction());

  functionTemplate.Reset(isolate, tpl);
}

void CharacteristicWrapper::New(const FunctionCallbackInfo<Value>& args) {
  constexpr int expectedArgs = 4;
  Isolate *isolate = Isolate::GetCurrent();
  if (args.Length() != expectedArgs) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }

  if (args.IsConstructCall()) {
    artik_bt_gatt_chr characteristic;
    std::vector<DescriptorWrapper*> descriptors;

    auto uuid = js_type_to_cpp<std::string>(args[0]);
    auto properties = js_type_to_cpp<std::vector<std::string>>(args[1]);
    auto js_descriptors = js_type_to_cpp<Local<Array>>(args[3]);

    if (!uuid || !properties || !js_descriptors) {
      isolate->ThrowException(Nan::TypeError("Wrong argument type"));
      return;
    }

    std::string error;
    characteristic.property = CharacteristicWrapper::ConvertStringProperties(
        properties.value(), &error);

    if (characteristic.property == -1) {
      Nan::TypeError(error.c_str());
      return;
    }

    characteristic.uuid = strdup(uuid.value().c_str());
    if (node::Buffer::HasInstance(args[2])) {
      unsigned char *val = (unsigned char*)node::Buffer::Data(args[2]);
      size_t len = node::Buffer::Length(args[2]);
      characteristic.value = (unsigned char*)malloc(sizeof(unsigned char)*len);
      characteristic.length = len;
      memcpy(characteristic.value, val, len);
    } else {
      characteristic.value = NULL;
      characteristic.length = 0;
    }

    Local<Array> array_descriptors = js_descriptors.value();
    for (unsigned int i = 0; i < array_descriptors->Length(); ++i) {
      Local<Value> js_descriptor =
          Nan::Get(array_descriptors, i).ToLocalChecked();
      if (!DescriptorWrapper::HasInstance(js_descriptor)) {
        isolate->ThrowException(Nan::TypeError("Wrong argument type"));
        return;
      }

      DescriptorWrapper* descriptor_wrap =
        ObjectWrap::Unwrap<DescriptorWrapper>(js_descriptor->ToObject());
      descriptors.push_back(descriptor_wrap);
    }

    CharacteristicWrapper* obj = new CharacteristicWrapper(characteristic,
        descriptors);
    obj->Wrap(args.This());

    Local<Function> emit =
      Local<Function>::Cast(obj->handle()->Get(
          String::NewFromUtf8(isolate, "emit")));
    obj->m_emit = new Persistent<Function>(isolate, emit);
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

int CharacteristicWrapper::ConvertStringProperties(
    const std::vector<std::string> &strv, std::string  *error) {
  std::array<const char *, 7> properties = {
    "broadcast",
    "read",
    "write-without-response",
    "write",
    "notify",
    "indicate"
    "signed-write"
  };
  std::array<int, 7> c_properties = {
    BT_GATT_CHAR_PROPERTY_BROADCAST,
    BT_GATT_CHAR_PROPERTY_READ,
    BT_GATT_CHAR_PROPERTY_WRITE_NO_RESPONSE,
    BT_GATT_CHAR_PROPERTY_WRITE,
    BT_GATT_CHAR_PROPERTY_NOTIFY,
    BT_GATT_CHAR_PROPERTY_INDICATE,
    BT_GATT_CHAR_PROPERTY_SIGNED_WRITE
  };

  int prop = 0;
  for (const auto& str : strv) {
    auto property = to_artik_parameter<artik_bt_gatt_char_properties>(
        properties, str.c_str());
    if (!property) {
      *error = "Property " + str + " is not supported";
      return -1;
    }
    prop |= c_properties[property.value()];
  }

  return prop;
}

CharacteristicWrapper::CharacteristicWrapper(
  const artik_bt_gatt_chr& characteristic,
  const std::vector<DescriptorWrapper*>& descriptors)
  : m_characteristic(characteristic), m_descriptors(descriptors) {
}

CharacteristicWrapper::~CharacteristicWrapper() {
  if (m_characteristic.value) {
    free(m_characteristic.value);
  }
  free(m_characteristic.uuid);
  delete m_emit;
}

void CharacteristicWrapper::Emit(int argc, Local<Value> argv[]) {
  Local<Function> emit_f = Nan::New<Function>(*m_emit);
  emit_f->Call(this->handle(), argc, argv);
}

void CharacteristicWrapper::SetIds(int service_id, int char_id) {
  m_service_id = service_id;
  m_characteristic_id = char_id;
}

int CharacteristicWrapper::GetServiceId() const {
  return m_service_id;
}

int CharacteristicWrapper::GetCharacteristicId() const {
  return m_characteristic_id;
}

void ServiceWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "Service"));

  Local<ObjectTemplate> objTpl = tpl->InstanceTemplate();
  objTpl->SetInternalFieldCount(1);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "Service"),
         tpl->GetFunction());
}

void ServiceWrapper::New(const FunctionCallbackInfo<Value>& args) {
  constexpr int expectedArgs = 2;
  Isolate *isolate = Isolate::GetCurrent();
  if (args.Length() != expectedArgs) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }

  if (args.IsConstructCall()) {
    artik_bt_gatt_service svc;
    std::vector<CharacteristicWrapper*> characteristics;

    auto svc_uuid = js_type_to_cpp<std::string>(args[0]);
    auto js_characteristics = js_type_to_cpp<Local<Array>>(args[1]);
    if (!svc_uuid || !js_characteristics) {
      isolate->ThrowException(Nan::TypeError("Wrong argument type"));
      return;
    }

    Local<Array> array_characteristics = js_characteristics.value();
    for (unsigned int i = 0; i < array_characteristics->Length(); ++i) {
      Local<Value> js_characteristic = Nan::Get(
          array_characteristics, i).ToLocalChecked();
      if (!CharacteristicWrapper::HasInstance(js_characteristic)) {
        isolate->ThrowException(Nan::TypeError("Wrong argument type"));
        return;
      }

      CharacteristicWrapper* characteristic_wrap =
          ObjectWrap::Unwrap<CharacteristicWrapper>(
              js_characteristic->ToObject());
      characteristics.push_back(characteristic_wrap);
    }

    svc.uuid = svc_uuid.value().c_str();
    // Bluez does not support secondary services
    svc.primary = true;

    ServiceWrapper* obj = new ServiceWrapper(svc, characteristics);
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

static void char_read_request(artik_bt_gatt_req request, void *user_data) {
  Nan::HandleScope scope;
  CharacteristicWrapper *wrap =
      reinterpret_cast<CharacteristicWrapper*>(user_data);

  log_dbg("ReadValue");
  Local<Value> argv[] = {
    Nan::New<String>("readRequest").ToLocalChecked(),
    Nan::New<External>(request)
  };

  log_dbg("Emit readRequest");
  wrap->Emit(2, argv);
}

static void char_write_request(artik_bt_gatt_req request,
    const unsigned char *val, int len, void *user_data) {
  Nan::HandleScope scope;
  CharacteristicWrapper *wrap =
      reinterpret_cast<CharacteristicWrapper*>(user_data);

  Local<Object> buffer = Nan::CopyBuffer((
        const char*)val, len).ToLocalChecked();
  Local<Value> argv[] = {
    Nan::New<String>("writeRequest").ToLocalChecked(),
    Nan::New<External>(request),
    buffer,
  };

  wrap->Emit(3, argv);
}

static void char_notify_request(bool state, void *user_data) {
  Nan::HandleScope scope;
  CharacteristicWrapper *wrap =
      reinterpret_cast<CharacteristicWrapper*>(user_data);

  log_dbg("char_id notify %d", wrap->GetCharacteristicId());
  Local<Value> argv[] = {
    Nan::New<String>("notifyRequest").ToLocalChecked(),
    Nan::New<Int32>(wrap->GetServiceId()),
    Nan::New<Int32>(wrap->GetCharacteristicId()),
    Nan::New<Boolean>(state)
  };

  wrap->Emit(4, argv);
}

static void desc_on_read_request(artik_bt_gatt_req request, void *user_data) {
  Nan::HandleScope scope;
  DescriptorWrapper *wrap = reinterpret_cast<DescriptorWrapper*>(user_data);

  Local<Value> argv[] = {
    Nan::New<String>("readRequest").ToLocalChecked(),
    Nan::New<External>(request)
  };

  wrap->Emit(2, argv);
}

static void desc_on_write_request(artik_bt_gatt_req request,
    const unsigned char *val, int len, void *user_data) {
  Nan::HandleScope scope;

  DescriptorWrapper *wrap = reinterpret_cast<DescriptorWrapper*>(user_data);
  Local<Object> buffer = Nan::CopyBuffer(
      (const char*)val, len).ToLocalChecked();
  Local<Value> argv[] = {
    Nan::New<String>("writeRequest").ToLocalChecked(),
    Nan::New<External>(request),
    buffer,
  };

  wrap->Emit(3, argv);
}

ServiceWrapper::ServiceWrapper(
  const artik_bt_gatt_service& service,
  const std::vector<CharacteristicWrapper*>& characteristics)
  : m_bt(new Bluetooth()),
    m_characteristics(characteristics) {
  m_bt->gatt_add_service(service, &m_service_id);
  for (auto& chr : m_characteristics) {
    int char_id;
    m_bt->gatt_add_characteristic(m_service_id, chr->GetChar(), &char_id);
    m_persistent_objects.push_back(chr->persistent());
    m_bt->gatt_set_char_on_read_request(m_service_id, char_id,
        char_read_request, reinterpret_cast<void*>(chr));
    m_bt->gatt_set_char_on_write_request(m_service_id, char_id,
        char_write_request, reinterpret_cast<void*>(chr));
    m_bt->gatt_set_char_on_notify_request(m_service_id, char_id,
        char_notify_request, reinterpret_cast<void*>(chr));

    chr->SetIds(m_service_id, char_id);
    for (auto& desc : chr->GetDescriptors()) {
      int desc_id;
      m_bt->gatt_add_descriptor(m_service_id, char_id, desc->GetDesc(),
          &desc_id);
      m_bt->gatt_set_desc_on_read_request(m_service_id, char_id, desc_id,
          desc_on_read_request, reinterpret_cast<void*>(desc));
      m_bt->gatt_set_desc_on_write_request(m_service_id, char_id, desc_id,
          desc_on_write_request, reinterpret_cast<void*>(desc));
      m_persistent_objects.push_back(desc->persistent());
      desc->SetId(desc_id);
    }
  }

  m_bt->gatt_register_service(m_service_id);
}

ServiceWrapper::~ServiceWrapper() {
  for (auto& chr : m_characteristics) {
    for (auto& desc : chr->GetDescriptors()) {
      m_bt->gatt_remove_descriptor(m_service_id, chr->GetCharacteristicId(),
          desc->GetDescriptorId());
    }
    m_bt->gatt_remove_characteristic(m_service_id, chr->GetCharacteristicId());
  }
  m_bt->gatt_unregister_service(m_service_id);
  m_bt->gatt_remove_service(m_service_id);
  delete m_bt;
}

GattServerWrapper::GattServerWrapper()
  : m_bt(new Bluetooth()) {
  m_loop = GlibLoop::Instance();
  m_loop->attach();
}

GattServerWrapper::~GattServerWrapper() {
  delete m_bt;
  m_loop->detach();
}

void GattServerWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "GattServer"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  NODE_SET_PROTOTYPE_METHOD(tpl, "request_send_value", request_send_value);
  NODE_SET_PROTOTYPE_METHOD(tpl, "request_send_result", request_send_result);
  NODE_SET_PROTOTYPE_METHOD(tpl, "start_advertising", start_advertising);
  NODE_SET_PROTOTYPE_METHOD(tpl, "stop_advertising", stop_advertising);
  NODE_SET_PROTOTYPE_METHOD(tpl, "notify", notify);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "GattServer"), tpl->GetFunction());
}

void GattServerWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = Isolate::GetCurrent();
  if (args.Length() != 0) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }

  if (args.IsConstructCall()) {
    GattServerWrapper* obj = new GattServerWrapper();
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

void GattServerWrapper::request_send_value(
    const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = Isolate::GetCurrent();
  Bluetooth* bt = ObjectWrap::Unwrap<GattServerWrapper>(
      args.Holder())->getObj();
  if (args.Length() != 2) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }

  if (!args[0]->IsExternal()) {
    isolate->ThrowException(Nan::TypeError("Wrong argument type"));
    return;
  }

  if (!node::Buffer::HasInstance(args[1])) {
    isolate->ThrowException(Nan::TypeError("Wrong argument type"));
    return;
  }

  Local<External> request = Local<External>::Cast(args[0]);
  unsigned char *val = (unsigned char*) node::Buffer::Data(args[1]);
  size_t len = node::Buffer::Length(args[1]);
  artik_bt_gatt_req *req = reinterpret_cast<artik_bt_gatt_req*>(
      request->Value());
  artik_error err = bt->gatt_req_set_value(req, len, val);

  if (err != S_OK) {
    std::string error = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Nan::TypeError(error.c_str()));
    return;
  }
}

void GattServerWrapper::request_send_result(
    const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = Isolate::GetCurrent();
  std::array<const char*, 7> states = {
    "ok",
    "failed",
    "inProgress",
    "notPermitted",
    "invalidValueLength",
    "notAuthorized",
    "notSupported"
  };

  Bluetooth* bt = ObjectWrap::Unwrap<GattServerWrapper>(
      args.Holder())->getObj();
  if (args.Length() != 3 && args.Length() != 2) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }

  if (!args[0]->IsExternal()) {
    isolate->ThrowException(Nan::TypeError("Wrong argument type"));
    return;
  }

  auto state = js_type_to_cpp<std::string>(args[1]);
  auto err_msg = js_type_to_cpp<std::string>(args[2]);

  if (!state) {
    isolate->ThrowException(Nan::TypeError("Wrong argument type"));
    return;
  }

  auto artik_state = to_artik_parameter<artik_bt_gatt_req_state_type>(
      states, state.value().c_str());

  if (!artik_state) {
    std::string error_msg = "State " + state.value() + "is not supported";
    isolate->ThrowException(Nan::TypeError(error_msg.c_str()));
    return;
  }

  const char *error = NULL;
  if (err_msg) {
    error = err_msg.value().c_str();
  }

  Local<External> request = Local<External>::Cast(args[0]);
  artik_bt_gatt_req *req = reinterpret_cast<artik_bt_gatt_req*>(
      request->Value());

  artik_error err = bt->gatt_req_set_result(req, artik_state.value(), error);
  if (err != S_OK) {
    std::string error = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Nan::TypeError(error.c_str()));
    return;
  }
}

void GattServerWrapper::start_advertising(
    const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = Isolate::GetCurrent();
  Bluetooth* bt = ObjectWrap::Unwrap<GattServerWrapper>(
      args.Holder())->getObj();
  if (args.Length() != 1) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }

  auto type = js_object_attribute_to_cpp<std::string>(args[0], "type");
  auto svc_uuid = js_object_attribute_to_cpp<std::vector<std::string>>(
      args[0], "serviceUuids");
  if (!type || !svc_uuid) {
    isolate->ThrowException(Nan::TypeError("Wrong argument type"));
    return;
  }

  static artik_bt_advertisement adv;
  memset(&adv, 0, sizeof(artik_bt_advertisement));
  adv.type = strdup(type.value().c_str());
  adv.svc_uuid = (const char **) malloc(
      sizeof(char *) * svc_uuid.value().size());
  adv.svc_uuid_len = svc_uuid.value().size();

  for (unsigned int i = 0; i < svc_uuid.value().size(); ++i) {
    adv.svc_uuid[i] = strdup(svc_uuid.value().at(i).c_str());
  }

  // optional fields
  auto solicit_uuid = js_object_attribute_to_cpp<std::vector<std::string>>(
      args[0], "solicitUuids");
  if (solicit_uuid) {
    adv.solicit_uuid = (const char **)malloc(
        sizeof(char *)* solicit_uuid.value().size());
    adv.solicit_uuid_len = solicit_uuid.value().size();
    for (unsigned int i = 0; i < solicit_uuid.value().size(); ++i) {
      adv.solicit_uuid[i] = strdup(solicit_uuid.value().at(i).c_str());
    }
  }

  auto include_tx_power = js_object_attribute_to_cpp<bool>(
      args[0], "includeTxPower");
  if (include_tx_power) {
    adv.tx_power = include_tx_power.value();
  }

  auto mfr_data = js_object_attribute_to_cpp<Local<Value>>(
      args[0], "manufacturerData");
  if (mfr_data) {
    auto id = js_object_attribute_to_cpp<uint16_t>(
        mfr_data.value(), "id");
    auto data = js_object_attribute_to_cpp<std::vector<unsigned char>>(
        mfr_data.value(), "data");
    if (!id || !data) {
      isolate->ThrowException(Nan::TypeError("Wrong argument type"));
      free(adv.svc_uuid);
      if (adv.solicit_uuid)
        free(adv.solicit_uuid);
      return;
    }

    adv.mfr_id = id.value();
    adv.mfr_data_len = data.value().size();
    adv.mfr_data = data.value().data();
  }

  auto service_data = js_object_attribute_to_cpp<Local<Value>>(
      args[0], "serviceData");
  if (service_data) {
    auto id = js_object_attribute_to_cpp<std::string>(
        service_data.value(), "id");
    auto data = js_object_attribute_to_cpp<std::vector<unsigned char>>(
        service_data.value(), "data");
    if (!id || !data) {
      isolate->ThrowException(
          Nan::New<String>("Wrong argument type").ToLocalChecked());
      free(adv.svc_uuid);
      if (adv.solicit_uuid)
        free(adv.solicit_uuid);
      return;
    }

    adv.svc_id = id.value().c_str();
    adv.svc_data_len = data.value().size();
    adv.svc_data = data.value().data();
  }

  int adv_id;
  artik_error err = bt->register_advertisement(&adv, &adv_id);

  if (err != S_OK) {
    std::string error = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Nan::TypeError(error.c_str()));
    return;
  }

  args.GetReturnValue().Set(Nan::New<Int32>(adv_id));
}

void GattServerWrapper::stop_advertising(
    const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = Isolate::GetCurrent();
  Bluetooth* bt = ObjectWrap::Unwrap<GattServerWrapper>(
      args.Holder())->getObj();
  if (args.Length() != 1) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }

  auto id = js_type_to_cpp<int>(args[0]);
  if (id) {
    isolate->ThrowException(Nan::TypeError("Wrong argument type"));
    return;
  }

  bt->unregister_advertisement(id.value());
}

void GattServerWrapper::notify(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = Isolate::GetCurrent();
  Bluetooth* bt = ObjectWrap::Unwrap<GattServerWrapper>(
      args.Holder())->getObj();
  if (args.Length() != 3) {
    isolate->ThrowException(Nan::TypeError("Wrong number of arguments"));
    return;
  }

  auto service_id = js_type_to_cpp<int>(args[0]);
  auto characteristic_id = js_type_to_cpp<int>(args[1]);

  if (!service_id || !characteristic_id) {
    isolate->ThrowException(Nan::TypeError("Wrong argument type"));
    return;
  }

  if (!node::Buffer::HasInstance(args[2])) {
    isolate->ThrowException(Nan::TypeError("Wrong argument type"));
    return;
  }

  unsigned char *val = (unsigned char*) node::Buffer::Data(args[2]);
  size_t len = node::Buffer::Length(args[2]);

  artik_error err = bt->gatt_notify(service_id.value(),
      characteristic_id.value(), val, len);
  if (err != S_OK) {
    std::string error = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Nan::TypeError(error.c_str()));
    return;
  }
}

}  // namespace artik
