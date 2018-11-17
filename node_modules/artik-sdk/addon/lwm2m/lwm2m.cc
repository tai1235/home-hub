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

#include "lwm2m/lwm2m.h"

#include <unistd.h>
#include <node_buffer.h>
#include <nan.h>
#include <artik_log.h>
#include <utils.h>

#include <string>
#include <vector>

#include "json.hpp"

#include "base/ssl_config_converter.h"

using json = nlohmann::json;

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
using v8::Array;
using v8::Context;

Persistent<Function> Lwm2mWrapper::constructor;

static void on_error(void *data, void *user_data) {
  artik_error err = (artik_error)(intptr_t)data;
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  Lwm2mWrapper* wrap = reinterpret_cast<Lwm2mWrapper*>(user_data);

  log_dbg("");

  if (!wrap->getErrorCb())
    return;

  Handle<Value> argv[] = {
    Handle<Value>(String::NewFromUtf8(isolate, error_msg(err))),
  };

  Local<Function>::New(isolate, *wrap->getErrorCb())->Call(
      isolate->GetCurrentContext()->Global(), 1, argv);
}

static void on_execute_resource(void *data, void *user_data) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  Lwm2mWrapper* wrap = reinterpret_cast<Lwm2mWrapper*>(user_data);

  log_dbg("");

  if (!wrap->getExecuteCb())
    return;

  artik_lwm2m_resource_t *res =
                          reinterpret_cast<artik_lwm2m_resource_t *>(data);

  Handle<Value> argv[] = {
    Handle<Value>(String::NewFromUtf8(isolate, res->uri))
  };

  Local<Function>::New(isolate, *wrap->getExecuteCb())->Call(
      isolate->GetCurrentContext()->Global(), 1, argv);
}

static void on_changed_resource(void *data, void *user_data) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  Lwm2mWrapper* wrap = reinterpret_cast<Lwm2mWrapper*>(user_data);

  log_dbg("");

  if (!wrap->getChangedCb())
    return;

  artik_lwm2m_resource_t *res =
                          reinterpret_cast<artik_lwm2m_resource_t *>(data);
  Local<Object> buffer = Nan::CopyBuffer((
        const char*)res->buffer, res->length).ToLocalChecked();

  Handle<Value> argv[] = {
    Handle<Value>(String::NewFromUtf8(isolate, res->uri)),
    buffer
  };

  Local<Function>::New(isolate, *wrap->getChangedCb())->Call(
      isolate->GetCurrentContext()->Global(), 2, argv);
}

Lwm2mWrapper::Lwm2mWrapper() {
  m_lwm2m = new Lwm2m();
  memset(&m_config, 0, sizeof(m_config));
  m_error_cb = NULL;
  m_execute_cb = NULL;
  m_changed_cb = NULL;
  m_loop = GlibLoop::Instance();
  m_loop->attach();
}

Lwm2mWrapper::~Lwm2mWrapper() {
  delete m_lwm2m;
  m_loop->detach();
}

void Lwm2mWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();
  Local<FunctionTemplate> modal = FunctionTemplate::New(isolate, New);

  log_dbg("");

  modal->SetClassName(String::NewFromUtf8(isolate, "lwm2m"));
  modal->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(modal, "client_request", client_request);
  NODE_SET_PROTOTYPE_METHOD(modal, "client_release", client_release);
  NODE_SET_PROTOTYPE_METHOD(modal, "client_connect", client_connect);
  NODE_SET_PROTOTYPE_METHOD(modal, "client_disconnect", client_disconnect);
  NODE_SET_PROTOTYPE_METHOD(modal, "client_write_resource",
                            client_write_resource);
  NODE_SET_PROTOTYPE_METHOD(modal, "client_read_resource",
                            client_read_resource);
  NODE_SET_PROTOTYPE_METHOD(modal, "serialize_tlv_int", serialize_tlv_int);
  NODE_SET_PROTOTYPE_METHOD(modal, "serialize_tlv_string",
                            serialize_tlv_string);

  constructor.Reset(isolate, modal->GetFunction());
  exports->Set(v8::String::NewFromUtf8(isolate, "lwm2m"),
     modal->GetFunction());
}


void Lwm2mWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  Lwm2mWrapper* obj = NULL;

  log_dbg("");

  if (args.IsConstructCall()) {
    obj = new Lwm2mWrapper();
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

void Lwm2mWrapper::client_request(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Lwm2mWrapper* wrap = ObjectWrap::Unwrap<Lwm2mWrapper>(args.Holder());
  Lwm2m* obj = wrap->getObj();
  artik_error ret = S_OK;

  log_dbg("");

  // Check Arguments
  if ((args.Length() < 5)  ||
      !args[0]->IsNumber() ||  // Server ID
      !args[1]->IsString() ||  // Server URI
      !args[2]->IsString() ||  // Client Name
      !args[3]->IsNumber() ||  // lifetime
      !args[4]->IsNumber() ||  // connect_timeout
      !args[5]->IsObject()) {  // Objects device & conn_monitoring
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }
  // Parse lwm2m object parameter and create appropriate object
  auto deviceobj = js_object_attribute_to_cpp<Local<Value>>(args[5], "device");
  auto connobj = js_object_attribute_to_cpp<Local<Value>>(
      args[5], "conn_monitoring");
  auto firmwareobj = js_object_attribute_to_cpp<Local<Value>>(
      args[5], "firmware");

  if (!deviceobj) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Missing entry in Objects: 'device' field must be provided")));
    return;
  }

  std::vector<std::string> check_deviceobj = {
      "", "manufacturer", "", "model",
      "", "serial",
      "", "fwVersion",
      "", "hwVersion",
      "", "swVersion",
      "", "deviceType",
      "", "powerSource",
      "", "powerVoltage",
      "", "powerCurrent",
      "", "batteryLevel",
      "", "memoryTotal",
      "", "memoryFree",
      "", "timeZone",
      "", "utcOffset",
      "", "bindingModes"
  };
  std::vector<std::string> check_connobj = {
      "", "netbearer",
      "", "avalnetbearer",
      "", "signalstrength",
      "", "linkquality",
      "", "ipaddr",
      "", "ipaddr2",
      "", "routeaddr",
      "", "routeaddr2",
      "", "linkutilization",
      "", "apn",
      "", "cellid",
      "", "smnc",
      "", "smcc"};

  std::vector<std::string> check_firmwareobj = {
      "", "pkgName",
      "", "pkgVersion"};

  for (unsigned int i = 0; i < check_deviceobj.size(); i += 2) {
    auto tmpDevObjVal = js_object_attribute_to_cpp<std::string>(
        deviceobj.value(), check_deviceobj[i+1]);
    if (!tmpDevObjVal || tmpDevObjVal.value().empty()) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
            isolate, ("Missing entry in device object: '" +
            check_deviceobj[i+1]+"'").c_str())));
      return;
    }
    check_deviceobj[i] = tmpDevObjVal.value();
    if (connobj && i < check_connobj.size()) {
      auto tmpConnObjVal = js_object_attribute_to_cpp<std::string>(
          connobj.value(), check_connobj[i+1]);
      if (!tmpConnObjVal ||
          (tmpConnObjVal.value().empty() && (i > 15 || i < 8))) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
            isolate, ("Missing entry in connectivity monitoring object: " +
            check_connobj[i+1]).c_str())));
        return;
      }
      check_connobj[i] = tmpConnObjVal.value();
    }
    if (firmwareobj && i < check_firmwareobj.size()) {
      auto tmpFirmObjVal = js_object_attribute_to_cpp<std::string>(
          firmwareobj.value(), check_firmwareobj[i+1]);
      if (!tmpFirmObjVal ||
          (tmpFirmObjVal.value().empty())) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
            isolate, ("Missing entry in firmware object: " +
            check_firmwareobj[i+1]).c_str())));
        return;
      }
      check_firmwareobj[i] = tmpFirmObjVal.value();
    }
  }

  // Fill the lwm2m config object
  v8::String::Utf8Value server_uri(args[1]->ToString());
  v8::String::Utf8Value client_name(args[2]->ToString());
  const char *ips[2] = {check_connobj[8].c_str(), check_connobj[10].c_str()};
  const char *routes[2] = {check_connobj[12].c_str(),
      check_connobj[14].c_str()};
  memset(&wrap->m_config, 0, sizeof(artik_lwm2m_config));
  wrap->m_config.server_id = args[0]->NumberValue();;
  wrap->m_config.server_uri = strndup(*server_uri, strlen(*server_uri));
  wrap->m_config.name = strndup(*client_name, strlen(*client_name));
  wrap->m_config.lifetime = args[3]->NumberValue();
  wrap->m_config.connect_timeout = args[4]->NumberValue();;

  // If TLS PSK or TLS CERT parameters are passed
  if ((args.Length() > 7) &&
       args[6]->IsString() &&
       args[7]->IsString()) {
    v8::String::Utf8Value identity(args[6]->ToString());
    v8::String::Utf8Value secret_key(args[7]->ToString());

    wrap->m_config.tls_psk_identity = strndup(*identity, strlen(*identity));
    wrap->m_config.tls_psk_key = strndup(*secret_key, strlen(*secret_key));

    if (args.Length() > 8 && args[8]->IsObject()) {
      wrap->m_config.ssl_config = reinterpret_cast<artik_ssl_config*>(malloc(
          sizeof(artik_ssl_config)));
      if (!wrap->m_config.ssl_config) {
          isolate->ThrowException(
                Exception::TypeError(String::NewFromUtf8(isolate,
                    "Failed to allocate memory for SSL config")));
          return;
      }
      memset(wrap->m_config.ssl_config, 0, sizeof(artik_ssl_config));

      auto se_cert_id_str =
        js_object_attribute_to_cpp<std::string>(args[8], "se_cert_id");
      auto client_cert =
        js_object_attribute_to_cpp<std::string>(args[8], "client_cert");
      auto client_private_key =
        js_object_attribute_to_cpp<std::string>(args[8],
                                                "client_private_key");

      if (se_cert_id_str) {
        auto cert_id =
          to_artik_parameter<artik_security_certificate_id>(
            SSLConfigConverter::security_certificate_ids,
            se_cert_id_str.value().c_str());
        if (!cert_id) {
          isolate->ThrowException(
                Exception::TypeError(String::NewFromUtf8(isolate,
                    "Wrong value of cert_id. ")));
          return;
        }
        wrap->m_config.ssl_config->se_config.use_se = true;
        wrap->m_config.ssl_config->se_config.certificate_id = cert_id.value();
      } else if (client_cert && client_private_key) {
        wrap->m_config.ssl_config->se_config.use_se = false;
        wrap->m_config.ssl_config->client_cert.data =
          strdup(client_cert.value().c_str());
        wrap->m_config.ssl_config->client_cert.len =
          client_cert.value().size();
        wrap->m_config.ssl_config->client_key.data =
          strdup(client_private_key.value().c_str());
        wrap->m_config.ssl_config->client_key.len =
          client_private_key.value().size();
      }

      auto server_cert =
        js_object_attribute_to_cpp<std::string>(args[8],
            "server_or_root_cert");
      if (server_cert) {
        wrap->m_config.ssl_config->ca_cert.data =
          strdup(server_cert.value().c_str());
        wrap->m_config.ssl_config->ca_cert.len = server_cert.value().size();
      }

      auto verify_cert =
        js_object_attribute_to_cpp<std::string>(args[8], "verify_cert");
      if (verify_cert) {
        if (verify_cert.value() == "none") {
          wrap->m_config.ssl_config->verify_cert = ARTIK_SSL_VERIFY_NONE;
        } else if (verify_cert.value() == "optional") {
          wrap->m_config.ssl_config->verify_cert = ARTIK_SSL_VERIFY_OPTIONAL;
        } else if (verify_cert.value() == "required") {
          wrap->m_config.ssl_config->verify_cert = ARTIK_SSL_VERIFY_REQUIRED;
        } else {
          isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
            isolate,
            "Wrong definition of verify_cert : expect 'none', 'optional' "
            "or 'required'.")));
          delete wrap->m_config.ssl_config;
          return;
        }
      } else {
        wrap->m_config.ssl_config->verify_cert = ARTIK_SSL_VERIFY_REQUIRED;
      }
    }
  }

  wrap->m_config.objects[ARTIK_LWM2M_OBJECT_DEVICE] = obj->create_device_object(
    check_deviceobj[0].c_str(), check_deviceobj[2].c_str(),
    check_deviceobj[4].c_str(), check_deviceobj[6].c_str(),
    check_deviceobj[8].c_str(), check_deviceobj[10].c_str(),
    check_deviceobj[12].c_str(), atoi(check_deviceobj[14].c_str()),
    atoi(check_deviceobj[16].c_str()), atoi(check_deviceobj[18].c_str()),
    atoi(check_deviceobj[20].c_str()), atoi(check_deviceobj[22].c_str()),
    atoi(check_deviceobj[24].c_str()), check_deviceobj[26].c_str(),
    check_deviceobj[28].c_str(), check_deviceobj[30].c_str());
  if (connobj) {
    wrap->m_config.objects[ARTIK_LWM2M_OBJECT_CONNECTIVITY_MONITORING] =
        obj->create_connectivity_monitoring_object(
            atoi(check_connobj[0].c_str()), atoi(check_connobj[2].c_str()),
            atoi(check_connobj[4].c_str()), atoi(check_connobj[6].c_str()),
            2, ips, 2, routes, atoi(check_connobj[16].c_str()),
            check_connobj[18].c_str(), atoi(check_connobj[20].c_str()),
            atoi(check_connobj[22].c_str()), atoi(check_connobj[24].c_str()));
  }

  if (firmwareobj) {
    char *pkgName = strdup(check_firmwareobj[0].c_str());
    char *pkgVersion = strdup(check_firmwareobj[2].c_str());
    wrap->m_config.objects[ARTIK_LWM2M_OBJECT_FIRMWARE] =
        obj->create_firmware_object(true, pkgName, pkgVersion);
    free(pkgName);
    free(pkgVersion);
  }

  ret = obj->client_request(&wrap->m_config);
  if (ret != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(ret));
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, msg.c_str())));
    return;
  }

  /* Set callbacks if passed as parameters */
  if (args.Length() > 9) {
    wrap->m_error_cb = new v8::Persistent<v8::Function>();
    wrap->m_error_cb->Reset(isolate, Local<Function>::Cast(args[9]));
    obj->set_callback(ARTIK_LWM2M_EVENT_ERROR, on_error,
        reinterpret_cast<void*>(wrap));
  }
  if (args.Length() > 10) {
    wrap->m_execute_cb = new v8::Persistent<v8::Function>();
    wrap->m_execute_cb->Reset(isolate, Local<Function>::Cast(args[10]));
    obj->set_callback(ARTIK_LWM2M_EVENT_RESOURCE_EXECUTE, on_execute_resource,
        reinterpret_cast<void*>(wrap));
  }
  if (args.Length() > 11) {
    wrap->m_changed_cb = new v8::Persistent<v8::Function>();
    wrap->m_changed_cb->Reset(isolate, Local<Function>::Cast(args[11]));
    obj->set_callback(ARTIK_LWM2M_EVENT_RESOURCE_CHANGED, on_changed_resource,
        reinterpret_cast<void*>(wrap));
  }
}

void Lwm2mWrapper::client_connect(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Lwm2mWrapper* wrap = ObjectWrap::Unwrap<Lwm2mWrapper>(args.Holder());
  Lwm2m* obj = wrap->getObj();
  artik_error ret = S_OK;

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  ret = obj->client_connect();

  if (ret != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(ret));
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, msg.c_str())));
    return;
  }
}

void Lwm2mWrapper::client_release(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Lwm2mWrapper* wrap = ObjectWrap::Unwrap<Lwm2mWrapper>(args.Holder());
  Lwm2m* obj = wrap->getObj();
  artik_error ret = S_OK;

  // Unset all registered callbacks
  if (wrap->m_error_cb) {
    obj->unset_callback(ARTIK_LWM2M_EVENT_ERROR);
    wrap->m_error_cb = NULL;
  }
  if (wrap->m_execute_cb) {
    obj->unset_callback(ARTIK_LWM2M_EVENT_RESOURCE_EXECUTE);
    wrap->m_execute_cb = NULL;
  }
  if (wrap->m_changed_cb) {
    obj->unset_callback(ARTIK_LWM2M_EVENT_RESOURCE_CHANGED);
    wrap->m_changed_cb = NULL;
  }

  ret = obj->client_release();
  if (ret != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(ret));
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, msg.c_str())));
    return;
  }

  // Free allocated resources
  obj->free_object(wrap->m_config.objects[ARTIK_LWM2M_OBJECT_DEVICE]);
  if (wrap->m_config.server_uri)
    free(wrap->m_config.server_uri);
  if (wrap->m_config.name)
    free(wrap->m_config.name);
  if (wrap->m_config.tls_psk_identity)
    free(wrap->m_config.tls_psk_identity);
  if (wrap->m_config.tls_psk_key)
    free(wrap->m_config.tls_psk_key);
}

void Lwm2mWrapper::client_disconnect(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Lwm2mWrapper* wrap = ObjectWrap::Unwrap<Lwm2mWrapper>(args.Holder());
  Lwm2m* obj = wrap->getObj();
  artik_error ret = S_OK;

  log_dbg("");

  // Disconnect the client
  ret = obj->client_disconnect();

  if (ret != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(ret));
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, msg.c_str())));
    return;
  }
}

void Lwm2mWrapper::client_write_resource(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Lwm2mWrapper* wrap = ObjectWrap::Unwrap<Lwm2mWrapper>(args.Holder());
  Lwm2m* obj = wrap->getObj();
  artik_error ret = S_OK;

  log_dbg("");

  // Check Arguments
  if ((args.Length() < 2) ||
      !args[0]->IsString()) {  // Resource URI
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  if (!node::Buffer::HasInstance(args[1])) {  // Data buffer
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Argument 2 should be a Buffer.")));
    return;
  }

  v8::String::Utf8Value uri(args[0]->ToString());
  unsigned char *buffer = (unsigned char *)node::Buffer::Data(args[1]);
  size_t length = node::Buffer::Length(args[1]);
  ret = obj->client_write_resource(*uri, buffer, length);

  if (ret != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(ret));
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, msg.c_str())));
    return;
  }
}

void Lwm2mWrapper::client_read_resource(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Lwm2mWrapper* wrap = ObjectWrap::Unwrap<Lwm2mWrapper>(args.Holder());
  Lwm2m* obj = wrap->getObj();
  artik_error ret = S_OK;
  char buffer[256]; int len = 256;

  log_dbg("");

  // Check Arguments
  if ((args.Length() < 1) || !args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value uri(args[0]->ToString());

  memset(buffer, 0, len);
  ret = obj->client_read_resource(*uri, (unsigned char*)buffer, &len);
  if (ret != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(ret));
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, msg.c_str())));
    return;
  }

  args.GetReturnValue().Set(Nan::CopyBuffer(buffer, len).ToLocalChecked());
}

void Lwm2mWrapper::serialize_tlv_int(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Lwm2mWrapper* wrap = ObjectWrap::Unwrap<Lwm2mWrapper>(args.Holder());
  Lwm2m* obj = wrap->getObj();
  artik_error ret = S_OK;
  unsigned char *buffer = NULL; int len = 0;

  log_dbg("");

  // Check Arguments
  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  v8::Array *data = v8::Array::Cast(*args[0]);
  int size = data->Length();
  int *rdata = new int(size);

  for (int i = 0; i < size; ++i)
    rdata[i] = data->Get(i)->NumberValue();

  ret = obj->serialize_tlv_int(rdata, size, &buffer, &len);
  if (ret != S_OK) {
    delete rdata;
    std::string msg = "Error: " + std::string(error_msg(ret));
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, msg.c_str())));
    return;
  }

  args.GetReturnValue().Set(Nan::CopyBuffer(
      (const char *)buffer, len).ToLocalChecked());

  delete buffer;
  delete rdata;
}

void Lwm2mWrapper::serialize_tlv_string(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Lwm2mWrapper* wrap = ObjectWrap::Unwrap<Lwm2mWrapper>(args.Holder());
  Lwm2m* obj = wrap->getObj();
  artik_error ret = S_OK;
  unsigned char *buffer = NULL;
  int len = 0;

  log_dbg("");

  // Check Arguments
  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  v8::Array *data = v8::Array::Cast(*args[0]);
  int size = data->Length();
  char *rdata[size];

  for (int i = 0; i < size; ++i) {
    String::Utf8Value val(data->Get(i)->ToString());
    rdata[i] = reinterpret_cast<char*>(*val);
  }

  ret = obj->serialize_tlv_string(rdata, size, &buffer, &len);
  if (ret != S_OK || !buffer) {
    std::string msg = "Error: " + std::string(error_msg(ret));
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, msg.c_str())));
    return;
  }

  args.GetReturnValue().Set(Nan::CopyBuffer(
      (const char *)buffer, len).ToLocalChecked());

  delete buffer;
}

}  // namespace artik
