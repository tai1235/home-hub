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

#include "wifi/wifi.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <artik_log.h>
#include <artik_types.h>

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
using v8::HandleScope;
using v8::Int32;
using v8::Boolean;
using v8::Context;

#define JSON_MAX_ENC_LEN  20

#define ARTIK_FREE(x) do { if (x) \
                             free(x);  \
                           x = NULL; \
                      } while (0)

Persistent<Function> WifiWrapper::constructor;

static char* convert_aps_to_json(artik_wifi_ap* aps, int num_aps) {
  char* json = NULL;
  int max_entry_len, max_json_len;
  char header[] = "\n[\n";
  char ap_entry[] = "\t{\n"
            "\t\t\"name\": \"%s\",\n"
            "\t\t\"bssid\": \"%s\",\n"
            "\t\t\"frequency\": %d,\n"
            "\t\t\"signal\": %d,\n"
            "\t\t\"encryption\": \"%s\",\n"
            "\t\t\"mode\": \"%s\"\n"
            "\t}\n";
  char footer[] = "]\n";

  // Compute max length for the JSON string and do the allocation
  max_entry_len = strlen(ap_entry)
          + MAX_AP_NAME_LEN     // name
          + MAX_AP_BSSID_LEN    // bssid
          + 11                  // frequency
          + 11                  // signal
          + JSON_MAX_ENC_LEN    // encryption
          + 16;                 // mode

  max_json_len = (max_entry_len*num_aps) + strlen(header) + strlen(footer) + 1;
  json = reinterpret_cast<char*>(malloc(max_json_len));
  if (!json)
    return json;

  // Start building the JSON string
  memset(json, 0, max_json_len);
  strncat(json, header, strlen(header));

  for (int i = 0; i < num_aps; i++) {
    char encryption[JSON_MAX_ENC_LEN];
    char* entry = reinterpret_cast<char*>(malloc(max_entry_len));
    if (!entry)
      break;

    memset(encryption, 0, JSON_MAX_ENC_LEN);
    memset(entry, 0, max_entry_len);

    if (aps[i].encryption_flags & WIFI_ENCRYPTION_WPA2)
      strncat(encryption, "WPA2,", JSON_MAX_ENC_LEN);
    if (aps[i].encryption_flags & WIFI_ENCRYPTION_WPA)
      strncat(encryption, "WPA,", JSON_MAX_ENC_LEN);
    if (aps[i].encryption_flags & WIFI_ENCRYPTION_WEP)
      strncat(encryption, "WEP,", JSON_MAX_ENC_LEN);
    if (aps[i].encryption_flags == WIFI_ENCRYPTION_OPEN)
      strncat(encryption, "OPEN,", JSON_MAX_ENC_LEN);

    snprintf(entry, max_entry_len, ap_entry,
        aps[i].name,
        aps[i].bssid,
        aps[i].frequency,
        aps[i].signal_level,
        encryption,
        (aps[i].encryption_flags & WIFI_ENCRYPTION_WPA2_PERSONAL) ?
            "personal" :
            (aps[i].encryption_flags & WIFI_ENCRYPTION_WPA2_ENTERPRISE) ?
            "enterprise" :
            "none");

    strncat(json, entry, max_entry_len);

    // If not last entry, add a ',' character
    if (i < (num_aps-1))
      strncat(json, ",", strlen(","));

    ARTIK_FREE(entry);
  }

  strncat(json, footer, strlen(footer));

  return json;
}

WifiWrapper::WifiWrapper() {
  setMode(ARTIK_WIFI_MODE_NONE);
  m_wifi = new Wifi();
  m_loop = GlibLoop::Instance();
  m_loop->attach();
}

WifiWrapper::~WifiWrapper() {
  m_wifi->deinit();
  delete m_wifi;
  m_loop->detach();
}

void WifiWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "wifi"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "connect", connect);
  NODE_SET_PROTOTYPE_METHOD(tpl, "disconnect", disconnect);

  NODE_SET_PROTOTYPE_METHOD(tpl, "scan_request", scan_request);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_scan_result", get_scan_result);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_info", get_info);
  NODE_SET_PROTOTYPE_METHOD(tpl, "start_ap", start_ap);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "wifi"),
               tpl->GetFunction());
}

void WifiWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  artik_wifi_mode_t wifi_mode = ARTIK_WIFI_MODE_NONE;

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (args[0]->IsString()) {
    auto wifi_type = js_type_to_cpp<std::string>(args[0]);

    if (wifi_type.value() == "ap") {
      wifi_mode = ARTIK_WIFI_MODE_AP;
    } else if (wifi_type.value() == "station") {
      wifi_mode = ARTIK_WIFI_MODE_STATION;
    } else {
      isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, "Wrong definition of wifi_mode :"
                                       " expect 'ap' or 'station'.")));
      return;
    }
  }

  if (args.IsConstructCall()) {
    WifiWrapper* obj = new WifiWrapper();
    obj->Wrap(args.This());
    Wifi* wifi = obj->getObj();
    wifi->init(wifi_mode);
    obj->setMode(wifi_mode);
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

void WifiWrapper::disconnect(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  WifiWrapper* wrap = ObjectWrap::Unwrap<WifiWrapper>(args.Holder());
  Wifi* obj = wrap->getObj();
  artik_error ret = S_OK;

  if (wrap->getMode() != ARTIK_WIFI_MODE_STATION) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "disconnect() works only in"
                                     " station mode")));
    return;
  }

  ret = obj->disconnect();

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, error_msg(ret)));
}

static void on_scan_callback(void *result, void *user_data) {
  Isolate * isolate = Isolate::GetCurrent();
  HandleScope handleScope(isolate);
  WifiWrapper* wrap = reinterpret_cast<WifiWrapper*>(user_data);
  Wifi* obj = wrap->getObj();
  artik_wifi_ap *wifi_aps = NULL;
  char* json_res = NULL;
  int num_aps = 0;
  artik_error err = *(reinterpret_cast<artik_error*>(result));

  log_dbg("");

  if (err != S_OK)
      return;

  if (!wrap->getScanCb())
    return;

  obj->get_scan_result(&wifi_aps, &num_aps);

  if (num_aps == 0 || !wifi_aps)
    json_res = NULL;
  else
    json_res = convert_aps_to_json(wifi_aps, num_aps);

  ARTIK_FREE(wifi_aps);

  Handle<Value> argv[] = {
    Handle<Value>(String::NewFromUtf8(isolate, json_res ? json_res : "[]")),
  };

  ARTIK_FREE(json_res);

  Local<Function>::New(isolate, *wrap->getScanCb())->Call(
      isolate->GetCurrentContext()->Global(), 1, argv);
}

void WifiWrapper::scan_request(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  WifiWrapper* wrap = ObjectWrap::Unwrap<WifiWrapper>(args.Holder());
  Wifi* obj = wrap->getObj();
  artik_error ret = S_OK;

  if (wrap->getMode() != ARTIK_WIFI_MODE_STATION) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "scan_request() works only in"
                                     " station mode")));
    return;
  }

  log_dbg("");

  if (!args[0]->IsFunction()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  wrap->m_scan_cb = new Persistent<Function>();
  wrap->m_scan_cb->Reset(isolate, Local<Function>::Cast(args[0]));
  obj->set_scan_result_callback(on_scan_callback,
      reinterpret_cast<void*>(wrap));
  ret = obj->scan_request();

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, error_msg(ret)));
}

static void on_connect_callback(void *result, void *user_data) {
  Isolate * isolate = Isolate::GetCurrent();
  HandleScope handleScope(isolate);
  WifiWrapper* wrap = reinterpret_cast<WifiWrapper*>(user_data);
  bool connected =
      reinterpret_cast<artik_wifi_connection_info*>(result)->connected;
  artik_error err =
      reinterpret_cast<artik_wifi_connection_info*>(result)->error;

  log_dbg("");

  if (err != S_OK)
    return;

  if (!wrap->getConnectCb())
    return;

  Handle<Value> argv[] = {
    Handle<Value>(Boolean::New(isolate, connected))
  };

  Local<Function>::New(isolate, *wrap->getConnectCb())->Call(
      isolate->GetCurrentContext()->Global(), 1, argv);
}

void WifiWrapper::connect(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  WifiWrapper* wrap = ObjectWrap::Unwrap<WifiWrapper>(args.Holder());
  Wifi* obj = wrap->getObj();
  artik_error ret = S_OK;

  if (wrap->getMode() != ARTIK_WIFI_MODE_STATION) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "connect() works only in"
                                     " station mode")));
    return;
  }

  log_dbg("");

  if (!args[0]->IsString() || !args[1]->IsString() || !args[2]->IsBoolean()
      || !args[3]->IsFunction()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  String::Utf8Value param0(args[0]->ToString());
  String::Utf8Value param1(args[1]->ToString());
  char *ssid = *param0;
  char *pwd = *param1;
  bool store = args[2]->BooleanValue();

  wrap->m_connect_cb = new Persistent<Function>();
  wrap->m_connect_cb->Reset(isolate, Local<Function>::Cast(args[3]));
  obj->set_connect_callback(on_connect_callback, reinterpret_cast<void*>(wrap));
  ret = obj->connect(ssid, pwd, store);

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, error_msg(ret)));
}

void WifiWrapper::get_scan_result(const FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  WifiWrapper* wrap = ObjectWrap::Unwrap<WifiWrapper>(args.Holder());
  Wifi* obj = wrap->getObj();
  artik_wifi_ap *wifi_aps = NULL;
  int num_aps = 0;
  char* json_res = NULL;

  if (wrap->getMode() != ARTIK_WIFI_MODE_STATION) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "get_scan_result() works only in"
                                     " station mode")));
    return;
  }

  log_dbg("");

  obj->get_scan_result(&wifi_aps, &num_aps);

  if (num_aps == 0 || !wifi_aps)
    json_res = NULL;
  else
    json_res = convert_aps_to_json(wifi_aps, num_aps);

  ARTIK_FREE(wifi_aps);

  args.GetReturnValue().Set(
      String::NewFromUtf8(isolate, json_res ? json_res : "null data"));

  ARTIK_FREE(json_res);
}

void WifiWrapper::get_info(const FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  WifiWrapper* wrap = ObjectWrap::Unwrap<WifiWrapper>(args.Holder());
  Wifi* obj = wrap->getObj();
  artik_wifi_ap wifi_ap;
  artik_wifi_connection_info info;
  char* json_res = NULL;
  const char result[] = "\t{\n"
      "\t\t\"connected\": %s,\n"
      "\t\t\"error\": %s,\n"
      "\t\t\"name\": \"%s\",\n"
      "\t\t\"bssid\": \"%s\",\n"
      "\t\t\"frequency\": %d,\n"
      "\t\t\"signal\": %d,\n"
      "\t\t\"encryption\": \"%s\",\n"
      "\t\t\"mode\": \"%s\"\n"
      "\t}\n";


  if (wrap->getMode() != ARTIK_WIFI_MODE_STATION) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "get_info() works only in"
                                     " station mode")));
    return;
  }

  log_dbg("");

  artik_error ret = S_OK;
  memset(&info, 0, sizeof(artik_wifi_connection_info));
  memset(&wifi_ap, 0, sizeof(artik_wifi_ap));
  ret = obj->get_info(&info, &wifi_ap);

  if (ret != S_OK) {
    json_res = NULL;
  } else {
    char encryption[JSON_MAX_ENC_LEN];
    memset(encryption, 0, JSON_MAX_ENC_LEN);

    if (info.connected) {
        if (wifi_ap.encryption_flags & WIFI_ENCRYPTION_WPA2)
            strncpy(encryption, "WPA2", JSON_MAX_ENC_LEN);
        if (wifi_ap.encryption_flags & WIFI_ENCRYPTION_WPA)
            strncpy(encryption, "WPA", JSON_MAX_ENC_LEN);
        if (wifi_ap.encryption_flags & WIFI_ENCRYPTION_WEP)
            strncpy(encryption, "WEP", JSON_MAX_ENC_LEN);
        if (wifi_ap.encryption_flags == WIFI_ENCRYPTION_OPEN)
            strncpy(encryption, "OPEN", JSON_MAX_ENC_LEN);
    }

    if (asprintf(&json_res, result,
            (info.connected) ? "true" : "false",
            (info.error != S_OK) ? "true" : "false",
            (info.connected && strlen(wifi_ap.name) > 0) ? wifi_ap.name:"null",
            (info.connected && strlen(wifi_ap.bssid) > 0)? wifi_ap.bssid:"null",
            (info.connected) ? wifi_ap.frequency : 0,
            (info.connected) ? wifi_ap.signal_level : 0,
            (info.connected) ? encryption : "null",
            (info.connected) ? "none" : "null") < 0) {
      ret = E_NO_MEM;
      json_res = NULL;
    }
  }

  args.GetReturnValue().Set(
      String::NewFromUtf8(isolate, json_res ? json_res : error_msg(ret)));

  ARTIK_FREE(json_res);
}

void WifiWrapper::start_ap(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  WifiWrapper* wrap = ObjectWrap::Unwrap<WifiWrapper>(args.Holder());
  Wifi* obj = wrap->getObj();

  if (wrap->getMode() != ARTIK_WIFI_MODE_AP) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "start_ap() works only in"
                                     " access point mode")));
    return;
  }

  if (args.Length() != 4
      || !args[0]->IsString()
      || !args[1]->IsString()
      || !args[2]->IsNumber()
      || !args[3]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  String::Utf8Value param0(args[0]->ToString());
  String::Utf8Value param1(args[1]->ToString());
  char *ssid = *param0;
  char *pass = *param1;
  unsigned int channel = args[2]->NumberValue();
  unsigned int encryption = args[3]->NumberValue();
  artik_error ret = S_OK;

  ret = obj->start_ap(ssid, pass, channel, encryption);

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, error_msg(ret)));
}

}  // namespace artik

