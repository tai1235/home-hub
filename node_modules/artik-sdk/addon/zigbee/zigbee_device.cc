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

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <artik_log.h>
#include <glib.h>

#include "zigbee/zigbee.h"
#include "zigbee/zigbee_device.h"
#include "zigbee/zigbee_util.h"

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
using v8::MaybeLocal;
using v8::Context;

Persistent<Function> OnOffLightWrapper::constructor;
Persistent<Function> OnOffSwitchWrapper::constructor;
Persistent<Function> LevelControlSwitchWrapper::constructor;
Persistent<Function> DimmableLightWrapper::constructor;
Persistent<Function> LightSensorWrapper::constructor;
Persistent<Function> RemoteControlWrapper::constructor;

OnOffLightWrapper::OnOffLightWrapper() :
    m_zbd(NULL) {
}

OnOffLightWrapper::~OnOffLightWrapper() {
  if (m_zbd)
    delete m_zbd;
}

void OnOffLightWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("Init");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "zigbee_onoff_light"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "groups_get_local_name_support",
                            groups_get_local_name_support);
  NODE_SET_PROTOTYPE_METHOD(tpl, "groups_set_local_name_support",
                            groups_set_local_name_support);
  NODE_SET_PROTOTYPE_METHOD(tpl, "onoff_get_value", onoff_get_value);
  NODE_SET_PROTOTYPE_METHOD(tpl, "ezmode_commissioning_target_start",
                            ezmode_commissioning_target_start);
  NODE_SET_PROTOTYPE_METHOD(tpl, "ezmode_commissioning_target_stop",
                            ezmode_commissioning_target_stop);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "zigbee_onoff_light"),
      tpl->GetFunction());
}

void OnOffLightWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (!args[0]->IsObject() || !args[1]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  if (args.IsConstructCall()) {
    Local<Value> apiobject = args[0]->ToObject()->Get(
        String::NewFromUtf8(isolate, "api"));
    ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(
        apiobject->ToObject());
    Zigbee* zb_obj = wrap->getObj();

    OnOffLightWrapper* obj = new OnOffLightWrapper();
    obj->m_zbd = zb_obj->get_onofflight_device(args[1]->Int32Value());

    Handle<Object> This = args.This();
    This->Set(String::NewFromUtf8(isolate, "profile_id"),
        Number::New(isolate, obj->m_zbd->get_profile_id()));
    This->Set(String::NewFromUtf8(isolate, "device_id"),
        Number::New(isolate, obj->m_zbd->get_device_id()));
    This->Set(String::NewFromUtf8(isolate, "handle"),
        Number::New(isolate, (intptr_t)(obj->m_zbd->get_handle())));
    This->Set(String::NewFromUtf8(isolate, "endpoint_id"),
        Number::New(isolate, args[1]->Int32Value()));

    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

/**
 * var value = onoff_get_value()
 */
void OnOffLightWrapper::onoff_get_value(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  OnOffLightWrapper* wrap = ObjectWrap::Unwrap<OnOffLightWrapper>(
      args.Holder());
  OnOffLightDevice* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_onoff_status value;

  log_dbg("onoff_get_value");

  ret = obj->onoff_get_value(&value);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  switch (value) {
  case ARTIK_ZIGBEE_ONOFF_OFF:
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, "off"));
    break;
  case ARTIK_ZIGBEE_ONOFF_ON:
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, "on"));
    break;
  case ARTIK_ZIGBEE_ONOFF_TOGGLE:
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, "toggle"));
    break;
  default:
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, "unknown"));
    log_err("unknown value(%d)", value);
    break;
  }
}

/**
 * var flag = groups_get_local_name_support()
 */
void OnOffLightWrapper::groups_get_local_name_support(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  OnOffLightWrapper* wrap = ObjectWrap::Unwrap<OnOffLightWrapper>(
      args.Holder());
  OnOffLightDevice* obj = wrap->getObj();
  artik_error ret;
  Local<Value> eid = args.This()->Get(
      String::NewFromUtf8(isolate, "endpoint_id"));

  log_dbg("groups_get_local_name_support");

  ret = obj->groups_get_local_name_support(eid->Int32Value());
  if (ret != S_OK && ret != E_NOT_SUPPORTED) {
    throw_error(isolate, ret);
    return;
  }

  if (ret == S_OK)
    args.GetReturnValue().Set(true);
  else
    args.GetReturnValue().Set(false);
}

/**
 * groups_set_local_name_support(true or false)
 */
void OnOffLightWrapper::groups_set_local_name_support(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  OnOffLightWrapper* wrap = ObjectWrap::Unwrap<OnOffLightWrapper>(
      args.Holder());
  OnOffLightDevice* obj = wrap->getObj();
  artik_error ret;

  log_dbg("groups_set_local_name_support");

  if (!args[0]->IsBoolean()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  ret = obj->groups_set_local_name_support(args[0]->BooleanValue());
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * ezmode_commissioning_target_start()
 */
void OnOffLightWrapper::ezmode_commissioning_target_start(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  OnOffLightWrapper* wrap = ObjectWrap::Unwrap<OnOffLightWrapper>(
      args.Holder());
  OnOffLightDevice* obj = wrap->getObj();
  artik_error ret;

  log_dbg("ezmode_commissioning_target_start");

  ret = obj->ezmode_commissioning_target_start();
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * ezmode_commissioning_target_stop()
 */
void OnOffLightWrapper::ezmode_commissioning_target_stop(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  OnOffLightWrapper* wrap = ObjectWrap::Unwrap<OnOffLightWrapper>(
      args.Holder());
  OnOffLightDevice* obj = wrap->getObj();
  artik_error ret;

  log_dbg("ezmode_commissioning_target_stop");

  ret = obj->ezmode_commissioning_target_stop();
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

OnOffSwitchWrapper::OnOffSwitchWrapper() :
    m_zbd(NULL) {
}

OnOffSwitchWrapper::~OnOffSwitchWrapper() {
  if (m_zbd)
    delete m_zbd;
}

void OnOffSwitchWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("Init");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "zigbee_onoff_switch"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "identify_request", identify_request);
  NODE_SET_PROTOTYPE_METHOD(tpl, "identify_get_remaining_time",
                            identify_get_remaining_time);
  NODE_SET_PROTOTYPE_METHOD(tpl, "onoff_command", onoff_command);
  NODE_SET_PROTOTYPE_METHOD(tpl, "ezmode_commissioning_initiator_start",
                            ezmode_commissioning_initiator_start);
  NODE_SET_PROTOTYPE_METHOD(tpl, "ezmode_commissioning_initiator_stop",
                            ezmode_commissioning_initiator_stop);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "zigbee_onoff_switch"),
      tpl->GetFunction());
}

void OnOffSwitchWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (!args[0]->IsObject() || !args[1]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  if (args.IsConstructCall()) {
    Local<Value> apiobject = args[0]->ToObject()->Get(
        String::NewFromUtf8(isolate, "api"));
    ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(
        apiobject->ToObject());
    Zigbee* zb_obj = wrap->getObj();

    OnOffSwitchWrapper* obj = new OnOffSwitchWrapper();
    obj->m_zbd = zb_obj->get_onoffswitch_device(args[1]->Int32Value());

    Handle<Object> This = args.This();
    This->Set(String::NewFromUtf8(isolate, "profile_id"),
        Number::New(isolate, obj->m_zbd->get_profile_id()));
    This->Set(String::NewFromUtf8(isolate, "device_id"),
        Number::New(isolate, obj->m_zbd->get_device_id()));
    This->Set(String::NewFromUtf8(isolate, "handle"),
        Number::New(isolate, (intptr_t)(obj->m_zbd->get_handle())));
    This->Set(String::NewFromUtf8(isolate, "endpoint_id"),
        Number::New(isolate, args[1]->Int32Value()));

    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

/**
 * onoff_command({
 *   node_id: N,
 *   endpoint_id: N,
 *   server_cluster: [...],
 *   client_cluster: [...]
 * }, 'on')
 */
void OnOffSwitchWrapper::onoff_command(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  OnOffSwitchWrapper* wrap = ObjectWrap::Unwrap<OnOffSwitchWrapper>(
      args.Holder());
  OnOffSwitchDevice* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_onoff_status status;
  artik_zigbee_endpoint endpoint;

  log_dbg("onoff_command");

  if (!args[0]->IsObject() || !args[1]->IsString()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[1]->ToString());
  char *command = *param0;

  if (!g_strcmp0(command, "on")) {
    status = ARTIK_ZIGBEE_ONOFF_ON;
  } else if (!g_strcmp0(command, "off")) {
    status = ARTIK_ZIGBEE_ONOFF_OFF;
  } else if (!g_strcmp0(command, "toggle")) {
    status = ARTIK_ZIGBEE_ONOFF_TOGGLE;
  } else {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  convert_jsobject_endpoint(isolate, args[0]->ToObject(), &endpoint);

  ret = obj->onoff_command(&endpoint, status);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * identify_request({
 *   node_id: N,
 *   endpoint_id: N,
 *   server_cluster: [...],
 *   client_cluster: [...]
 * }, 10)
 */
void OnOffSwitchWrapper::identify_request(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  OnOffSwitchWrapper* wrap = ObjectWrap::Unwrap<OnOffSwitchWrapper>(
      args.Holder());
  OnOffSwitchDevice* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_endpoint endpoint;

  log_dbg("identify_request");

  if (!args[0]->IsObject() || !args[1]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  convert_jsobject_endpoint(isolate, args[0]->ToObject(), &endpoint);

  ret = obj->identify_request(&endpoint, args[1]->Int32Value());
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * var remaining_time = identify_get_remaining_time({
 *   node_id: N,
 *   endpoint_id: N,
 *   server_cluster: [...],
 *   client_cluster: [...]
 * })
 */
void OnOffSwitchWrapper::identify_get_remaining_time(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  OnOffSwitchWrapper* wrap = ObjectWrap::Unwrap<OnOffSwitchWrapper>(
      args.Holder());
  OnOffSwitchDevice* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_endpoint endpoint;
  int remaining_time = 0;

  log_dbg("identify_get_remaining_time");

  if (!args[0]->IsObject()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  convert_jsobject_endpoint(isolate, args[0]->ToObject(), &endpoint);

  ret = obj->identify_get_remaining_time(&endpoint, &remaining_time);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Int32::New(isolate, remaining_time));
}

/**
 * ezmode_commissioning_initiator_start()
 */
void OnOffSwitchWrapper::ezmode_commissioning_initiator_start(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  OnOffSwitchWrapper* wrap = ObjectWrap::Unwrap<OnOffSwitchWrapper>(
      args.Holder());
  OnOffSwitchDevice* obj = wrap->getObj();
  artik_error ret;

  log_dbg("ezmode_commissioning_initiator_start");

  ret = obj->ezmode_commissioning_initiator_start();
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * ezmode_commissioning_initiator_stop()
 */
void OnOffSwitchWrapper::ezmode_commissioning_initiator_stop(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  OnOffSwitchWrapper* wrap = ObjectWrap::Unwrap<OnOffSwitchWrapper>(
      args.Holder());
  OnOffSwitchDevice* obj = wrap->getObj();
  artik_error ret;

  log_dbg("ezmode_commissioning_initiator_stop");

  ret = obj->ezmode_commissioning_initiator_stop();
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

LevelControlSwitchWrapper::LevelControlSwitchWrapper() :
    m_zbd(NULL) {
}

LevelControlSwitchWrapper::~LevelControlSwitchWrapper() {
  if (m_zbd)
    delete m_zbd;
}

void LevelControlSwitchWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("Init");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(
      String::NewFromUtf8(isolate, "zigbee_levelcontrol_switch"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "identify_request", identify_request);
  NODE_SET_PROTOTYPE_METHOD(tpl, "identify_get_remaining_time",
                            identify_get_remaining_time);
  NODE_SET_PROTOTYPE_METHOD(tpl, "onoff_command", onoff_command);
  NODE_SET_PROTOTYPE_METHOD(tpl, "level_control_request",
                            level_control_request);
  NODE_SET_PROTOTYPE_METHOD(tpl, "ezmode_commissioning_initiator_start",
                            ezmode_commissioning_initiator_start);
  NODE_SET_PROTOTYPE_METHOD(tpl, "ezmode_commissioning_initiator_stop",
                            ezmode_commissioning_initiator_stop);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "zigbee_levelcontrol_switch"),
      tpl->GetFunction());
}

void LevelControlSwitchWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (!args[0]->IsObject() || !args[1]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  if (args.IsConstructCall()) {
    Local<Value> apiobject = args[0]->ToObject()->Get(
        String::NewFromUtf8(isolate, "api"));
    ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(
        apiobject->ToObject());
    Zigbee* zb_obj = wrap->getObj();

    LevelControlSwitchWrapper* obj = new LevelControlSwitchWrapper();
    obj->m_zbd = zb_obj->get_levelcontrolswitch_device(
        args[1]->Int32Value());

    Handle<Object> This = args.This();
    This->Set(String::NewFromUtf8(isolate, "profile_id"),
        Number::New(isolate, obj->m_zbd->get_profile_id()));
    This->Set(String::NewFromUtf8(isolate, "device_id"),
        Number::New(isolate, obj->m_zbd->get_device_id()));
    This->Set(String::NewFromUtf8(isolate, "handle"),
        Number::New(isolate, (intptr_t)(obj->m_zbd->get_handle())));
    This->Set(String::NewFromUtf8(isolate, "endpoint_id"),
        Number::New(isolate, args[1]->Int32Value()));

    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

/**
 * onoff_command({
 *   node_id: N,
 *   endpoint_id: N,
 *   server_cluster: [...],
 *   client_cluster: [...]
 * }, 'on')
 */
void LevelControlSwitchWrapper::onoff_command(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  LevelControlSwitchWrapper* wrap = ObjectWrap::Unwrap<
      LevelControlSwitchWrapper>(args.Holder());
  LevelControlSwitchDevice* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_onoff_status status;
  artik_zigbee_endpoint endpoint;

  log_dbg("onoff_command");

  if (!args[0]->IsObject() || !args[1]->IsString()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[1]->ToString());
  char *command = *param0;

  if (!g_strcmp0(command, "on")) {
    status = ARTIK_ZIGBEE_ONOFF_ON;
  } else if (!g_strcmp0(command, "off")) {
    status = ARTIK_ZIGBEE_ONOFF_OFF;
  } else if (!g_strcmp0(command, "toggle")) {
    status = ARTIK_ZIGBEE_ONOFF_TOGGLE;
  } else {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  convert_jsobject_endpoint(isolate, args[0]->ToObject(), &endpoint);

  ret = obj->onoff_command(&endpoint, status);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * level_control_request({
 *   node_id: N,
 *   endpoint_id: N,
 *   server_cluster: [...],
 *   client_cluster: [...]
 * }, {
 *   type: 'moveup',
 *   value: 1,
 *   auto_onoff: true
 * })
 */
void LevelControlSwitchWrapper::level_control_request(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  LevelControlSwitchWrapper* wrap = ObjectWrap::Unwrap<
      LevelControlSwitchWrapper>(args.Holder());
  LevelControlSwitchDevice* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_level_control_command level;
  artik_zigbee_endpoint endpoint;

  log_dbg("level_control_request");

  if (!args[0]->IsObject() || !args[1]->IsObject()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  if (convert_jsobject_levelcontrol(isolate, args[1]->ToObject(), &level)
      < 0) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  convert_jsobject_endpoint(isolate, args[0]->ToObject(), &endpoint);

  ret = obj->level_control_request(&endpoint, &level);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * identify_request({
 *   node_id: N,
 *   endpoint_id: N,
 *   server_cluster: [...],
 *   client_cluster: [...]
 * }, 10)
 */
void LevelControlSwitchWrapper::identify_request(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  LevelControlSwitchWrapper* wrap = ObjectWrap::Unwrap<
      LevelControlSwitchWrapper>(args.Holder());
  LevelControlSwitchDevice* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_endpoint endpoint;

  log_dbg("identify_request");

  if (!args[0]->IsObject() || !args[1]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  convert_jsobject_endpoint(isolate, args[0]->ToObject(), &endpoint);

  ret = obj->identify_request(&endpoint, args[1]->Int32Value());
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * var remaining_time = identify_get_remaining_time({
 *   node_id: N,
 *   endpoint_id: N,
 *   server_cluster: [...],
 *   client_cluster: [...]
 * })
 */
void LevelControlSwitchWrapper::identify_get_remaining_time(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  LevelControlSwitchWrapper* wrap = ObjectWrap::Unwrap<
      LevelControlSwitchWrapper>(args.Holder());
  LevelControlSwitchDevice* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_endpoint endpoint;
  int remaining_time = 0;

  log_dbg("identify_get_remaining_time");

  if (!args[0]->IsObject()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  convert_jsobject_endpoint(isolate, args[0]->ToObject(), &endpoint);

  ret = obj->identify_get_remaining_time(&endpoint, &remaining_time);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Int32::New(isolate, remaining_time));
}

/**
 * ezmode_commissioning_initiator_start()
 */
void LevelControlSwitchWrapper::ezmode_commissioning_initiator_start(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  LevelControlSwitchWrapper* wrap = ObjectWrap::Unwrap<
      LevelControlSwitchWrapper>(args.Holder());
  LevelControlSwitchDevice* obj = wrap->getObj();
  artik_error ret;

  log_dbg("ezmode_commissioning_initiator_start");

  ret = obj->ezmode_commissioning_initiator_start();
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * ezmode_commissioning_initiator_stop()
 */
void LevelControlSwitchWrapper::ezmode_commissioning_initiator_stop(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  LevelControlSwitchWrapper* wrap = ObjectWrap::Unwrap<
      LevelControlSwitchWrapper>(args.Holder());
  LevelControlSwitchDevice* obj = wrap->getObj();
  artik_error ret;

  log_dbg("ezmode_commissioning_initiator_stop");

  ret = obj->ezmode_commissioning_initiator_stop();
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

DimmableLightWrapper::DimmableLightWrapper() :
    m_zbd(NULL) {
}

DimmableLightWrapper::~DimmableLightWrapper() {
  if (m_zbd)
    delete m_zbd;
}

void DimmableLightWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("Init");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "zigbee_dimmable_light"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "groups_get_local_name_support",
                            groups_get_local_name_support);
  NODE_SET_PROTOTYPE_METHOD(tpl, "groups_set_local_name_support",
                            groups_set_local_name_support);
  NODE_SET_PROTOTYPE_METHOD(tpl, "onoff_get_value",
                            onoff_get_value);
  NODE_SET_PROTOTYPE_METHOD(tpl, "level_control_get_value",
                            level_control_get_value);
  NODE_SET_PROTOTYPE_METHOD(tpl, "ezmode_commissioning_target_start",
                            ezmode_commissioning_target_start);
  NODE_SET_PROTOTYPE_METHOD(tpl, "ezmode_commissioning_target_stop",
                            ezmode_commissioning_target_stop);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "zigbee_dimmable_light"),
      tpl->GetFunction());
}

void DimmableLightWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (!args[0]->IsObject() || !args[1]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  if (args.IsConstructCall()) {
    Local<Value> apiobject = args[0]->ToObject()->Get(
        String::NewFromUtf8(isolate, "api"));
    ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(
        apiobject->ToObject());
    Zigbee* zb_obj = wrap->getObj();

    DimmableLightWrapper* obj = new DimmableLightWrapper();
    obj->m_zbd = zb_obj->get_dimmablelight_device(args[1]->Int32Value());

    Handle<Object> This = args.This();
    This->Set(String::NewFromUtf8(isolate, "profile_id"),
        Number::New(isolate, obj->m_zbd->get_profile_id()));
    This->Set(String::NewFromUtf8(isolate, "device_id"),
        Number::New(isolate, obj->m_zbd->get_device_id()));
    This->Set(String::NewFromUtf8(isolate, "handle"),
        Number::New(isolate, (intptr_t)(obj->m_zbd->get_handle())));
    This->Set(String::NewFromUtf8(isolate, "endpoint_id"),
        Number::New(isolate, args[1]->Int32Value()));

    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

/**
 * var value = onoff_get_value()
 */
void DimmableLightWrapper::onoff_get_value(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  DimmableLightWrapper* wrap = ObjectWrap::Unwrap<DimmableLightWrapper>(
      args.Holder());
  DimmableLightDevice* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_onoff_status value;

  log_dbg("onoff_get_value");

  ret = obj->onoff_get_value(&value);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  switch (value) {
  case ARTIK_ZIGBEE_ONOFF_OFF:
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, "off"));
    break;
  case ARTIK_ZIGBEE_ONOFF_ON:
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, "on"));
    break;
  case ARTIK_ZIGBEE_ONOFF_TOGGLE:
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, "toggle"));
    break;
  default:
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, "unknown"));
    log_err("unknown value(%d)", value);
    break;
  }
}

/**
 * var value = level_control_get_value()
 */
void DimmableLightWrapper::level_control_get_value(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  DimmableLightWrapper* wrap = ObjectWrap::Unwrap<DimmableLightWrapper>(
      args.Holder());
  DimmableLightDevice* obj = wrap->getObj();
  artik_error ret;
  int value;

  log_dbg("level_control_get_value");

  ret = obj->level_control_get_value(&value);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Int32::New(isolate, value));
}

/**
 * var flag = groups_get_local_name_support()
 */
void DimmableLightWrapper::groups_get_local_name_support(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  DimmableLightWrapper* wrap = ObjectWrap::Unwrap<DimmableLightWrapper>(
      args.Holder());
  DimmableLightDevice* obj = wrap->getObj();
  artik_error ret;
  Local<Value> eid = args.This()->Get(
      String::NewFromUtf8(isolate, "endpoint_id"));

  log_dbg("groups_get_local_name_support");

  ret = obj->groups_get_local_name_support(eid->Int32Value());
  if (ret != S_OK && ret != E_NOT_SUPPORTED) {
    throw_error(isolate, ret);
    return;
  }

  if (ret == S_OK)
    args.GetReturnValue().Set(true);
  else
    args.GetReturnValue().Set(false);
}

/**
 * groups_set_local_name_support(true or false)
 */
void DimmableLightWrapper::groups_set_local_name_support(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  DimmableLightWrapper* wrap = ObjectWrap::Unwrap<DimmableLightWrapper>(
      args.Holder());
  DimmableLightDevice* obj = wrap->getObj();
  artik_error ret;

  log_dbg("groups_set_local_name_support");

  if (!args[0]->IsBoolean()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  ret = obj->groups_set_local_name_support(args[0]->BooleanValue());
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * ezmode_commissioning_target_start()
 */
void DimmableLightWrapper::ezmode_commissioning_target_start(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  DimmableLightWrapper* wrap = ObjectWrap::Unwrap<DimmableLightWrapper>(
      args.Holder());
  DimmableLightDevice* obj = wrap->getObj();
  artik_error ret;

  log_dbg("ezmode_commissioning_target_start");

  ret = obj->ezmode_commissioning_target_start();
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * ezmode_commissioning_target_stop()
 */
void DimmableLightWrapper::ezmode_commissioning_target_stop(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  DimmableLightWrapper* wrap = ObjectWrap::Unwrap<DimmableLightWrapper>(
      args.Holder());
  DimmableLightDevice* obj = wrap->getObj();
  artik_error ret;

  log_dbg("ezmode_commissioning_target_stop");

  ret = obj->ezmode_commissioning_target_stop();
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

LightSensorWrapper::LightSensorWrapper() :
    m_zbd(NULL) {
}

LightSensorWrapper::~LightSensorWrapper() {
  if (m_zbd)
    delete m_zbd;
}

void LightSensorWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("Init");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "zigbee_light_sensor"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "identify_request",
                            identify_request);
  NODE_SET_PROTOTYPE_METHOD(tpl, "identify_get_remaining_time",
                            identify_get_remaining_time);
  NODE_SET_PROTOTYPE_METHOD(tpl, "illum_set_measured_value_range",
                            illum_set_measured_value_range);
  NODE_SET_PROTOTYPE_METHOD(tpl, "illum_set_measured_value",
                            illum_set_measured_value);
  NODE_SET_PROTOTYPE_METHOD(tpl, "illum_get_measured_value",
                            illum_get_measured_value);
  NODE_SET_PROTOTYPE_METHOD(tpl, "ezmode_commissioning_initiator_start",
                            ezmode_commissioning_initiator_start);
  NODE_SET_PROTOTYPE_METHOD(tpl, "ezmode_commissioning_initiator_stop",
                            ezmode_commissioning_initiator_stop);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "zigbee_light_sensor"),
      tpl->GetFunction());
}

void LightSensorWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (!args[0]->IsObject() || !args[1]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  if (args.IsConstructCall()) {
    Local<Value> apiobject = args[0]->ToObject()->Get(
        String::NewFromUtf8(isolate, "api"));
    ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(
        apiobject->ToObject());
    Zigbee* zb_obj = wrap->getObj();

    LightSensorWrapper* obj = new LightSensorWrapper();
    obj->m_zbd = zb_obj->get_lightsensor_device(args[1]->Int32Value());

    Handle<Object> This = args.This();
    This->Set(String::NewFromUtf8(isolate, "profile_id"),
        Number::New(isolate, obj->m_zbd->get_profile_id()));
    This->Set(String::NewFromUtf8(isolate, "device_id"),
        Number::New(isolate, obj->m_zbd->get_device_id()));
    This->Set(String::NewFromUtf8(isolate, "handle"),
        Number::New(isolate, (intptr_t)(obj->m_zbd->get_handle())));
    This->Set(String::NewFromUtf8(isolate, "endpoint_id"),
        Number::New(isolate, args[1]->Int32Value()));

    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

/**
 * illum_set_measured_value_range(min, max)
 */
void LightSensorWrapper::illum_set_measured_value_range(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  LightSensorWrapper* wrap = ObjectWrap::Unwrap<LightSensorWrapper>(
      args.Holder());
  LightSensorDevice* obj = wrap->getObj();
  artik_error ret;

  log_dbg("illum_set_measured_value_range");

  if (!args[0]->IsInt32() || !args[1]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  ret = obj->illum_set_measured_value_range(args[0]->Int32Value(),
      args[1]->Int32Value());
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * illum_set_measured_value(value)
 */
void LightSensorWrapper::illum_set_measured_value(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  LightSensorWrapper* wrap = ObjectWrap::Unwrap<LightSensorWrapper>(
      args.Holder());
  LightSensorDevice* obj = wrap->getObj();
  artik_error ret;

  log_dbg("illum_set_measured_value");

  if (!args[0]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  ret = obj->illum_set_measured_value(args[0]->Int32Value());
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * var value = illum_get_measured_value()
 */
void LightSensorWrapper::illum_get_measured_value(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  LightSensorWrapper* wrap = ObjectWrap::Unwrap<LightSensorWrapper>(
      args.Holder());
  LightSensorDevice* obj = wrap->getObj();
  artik_error ret;
  int value = 0;

  log_dbg("illum_get_measured_value");

  ret = obj->illum_get_measured_value(&value);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Int32::New(isolate, value));
}

/**
 * identify_request({
 *   node_id: N,
 *   endpoint_id: N,
 *   server_cluster: [...],
 *   client_cluster: [...]
 * }, 10)
 */
void LightSensorWrapper::identify_request(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  LightSensorWrapper* wrap = ObjectWrap::Unwrap<LightSensorWrapper>(
      args.Holder());
  LightSensorDevice* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_endpoint endpoint;

  log_dbg("identify_request");

  if (!args[0]->IsObject() || !args[1]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  convert_jsobject_endpoint(isolate, args[0]->ToObject(), &endpoint);

  ret = obj->identify_request(&endpoint, args[1]->Int32Value());
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * var remaining_time = identify_get_remaining_time({
 *   node_id: N,
 *   endpoint_id: N,
 *   server_cluster: [...],
 *   client_cluster: [...]
 * })
 */
void LightSensorWrapper::identify_get_remaining_time(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  LightSensorWrapper* wrap = ObjectWrap::Unwrap<LightSensorWrapper>(
      args.Holder());
  LightSensorDevice* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_endpoint endpoint;
  int remaining_time = 0;

  log_dbg("identify_get_remaining_time");

  if (!args[0]->IsObject()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  convert_jsobject_endpoint(isolate, args[0]->ToObject(), &endpoint);

  ret = obj->identify_get_remaining_time(&endpoint, &remaining_time);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Int32::New(isolate, remaining_time));
}

/**
 * ezmode_commissioning_initiator_start()
 */
void LightSensorWrapper::ezmode_commissioning_initiator_start(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  LightSensorWrapper* wrap = ObjectWrap::Unwrap<LightSensorWrapper>(
      args.Holder());
  LightSensorDevice* obj = wrap->getObj();
  artik_error ret;

  log_dbg("ezmode_commissioning_initiator_start");

  ret = obj->ezmode_commissioning_initiator_start();
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * ezmode_commissioning_initiator_stop()
 */
void LightSensorWrapper::ezmode_commissioning_initiator_stop(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  LightSensorWrapper* wrap = ObjectWrap::Unwrap<LightSensorWrapper>(
      args.Holder());
  LightSensorDevice* obj = wrap->getObj();
  artik_error ret;

  log_dbg("ezmode_commissioning_initiator_stop");

  ret = obj->ezmode_commissioning_initiator_stop();
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

RemoteControlWrapper::RemoteControlWrapper() :
    m_zbd(NULL) {
}

RemoteControlWrapper::~RemoteControlWrapper() {
  if (m_zbd)
    delete m_zbd;
}

void RemoteControlWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("Init");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "zigbee_remote_control"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "reset_to_factory_default",
                            reset_to_factory_default);
  NODE_SET_PROTOTYPE_METHOD(tpl, "identify_request",
                            identify_request);
  NODE_SET_PROTOTYPE_METHOD(tpl, "identify_get_remaining_time",
                            identify_get_remaining_time);
  NODE_SET_PROTOTYPE_METHOD(tpl, "onoff_command", onoff_command);
  NODE_SET_PROTOTYPE_METHOD(tpl, "level_control_request",
                            level_control_request);
  NODE_SET_PROTOTYPE_METHOD(tpl, "request_reporting", request_reporting);
  NODE_SET_PROTOTYPE_METHOD(tpl, "stop_reporting", stop_reporting);
  NODE_SET_PROTOTYPE_METHOD(tpl, "ezmode_commissioning_target_start",
                            ezmode_commissioning_target_start);
  NODE_SET_PROTOTYPE_METHOD(tpl, "ezmode_commissioning_target_stop",
                            ezmode_commissioning_target_stop);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "zigbee_remote_control"),
      tpl->GetFunction());
}

void RemoteControlWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (!args[0]->IsObject() || !args[1]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  if (args.IsConstructCall()) {
    Local<Value> apiobject = args[0]->ToObject()->Get(
        String::NewFromUtf8(isolate, "api"));
    ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(
        apiobject->ToObject());
    Zigbee* zb_obj = wrap->getObj();

    RemoteControlWrapper* obj = new RemoteControlWrapper();
    obj->m_zbd = zb_obj->get_remotecontrol_device(args[1]->Int32Value());

    Handle<Object> This = args.This();
    This->Set(String::NewFromUtf8(isolate, "profile_id"),
        Number::New(isolate, obj->m_zbd->get_profile_id()));
    This->Set(String::NewFromUtf8(isolate, "device_id"),
        Number::New(isolate, obj->m_zbd->get_device_id()));
    This->Set(String::NewFromUtf8(isolate, "handle"),
        Number::New(isolate, (intptr_t)(obj->m_zbd->get_handle())));
    This->Set(String::NewFromUtf8(isolate, "endpoint_id"),
        Number::New(isolate, args[1]->Int32Value()));

    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

/**
 * onoff_command({
 *   node_id: N,
 *   endpoint_id: N,
 *   server_cluster: [...],
 *   client_cluster: [...]
 * }, 'on')
 */
void RemoteControlWrapper::onoff_command(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  RemoteControlWrapper* wrap = ObjectWrap::Unwrap<
      RemoteControlWrapper>(args.Holder());
  RemoteControlDevice* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_onoff_status status;
  artik_zigbee_endpoint endpoint;

  log_dbg("onoff_command");

  if (!args[0]->IsObject() || !args[1]->IsString()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[1]->ToString());
  char *command = *param0;

  if (!g_strcmp0(command, "on")) {
    status = ARTIK_ZIGBEE_ONOFF_ON;
  } else if (!g_strcmp0(command, "off")) {
    status = ARTIK_ZIGBEE_ONOFF_OFF;
  } else if (!g_strcmp0(command, "toggle")) {
    status = ARTIK_ZIGBEE_ONOFF_TOGGLE;
  } else {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  convert_jsobject_endpoint(isolate, args[0]->ToObject(), &endpoint);

  ret = obj->onoff_command(&endpoint, status);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * level_control_request({
 *   node_id: N,
 *   endpoint_id: N,
 *   server_cluster: [...],
 *   client_cluster: [...]
 * }, {
 *   type: 'moveup',
 *   value: 1,
 *   auto_onoff: true
 * })
 */
void RemoteControlWrapper::level_control_request(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  RemoteControlWrapper* wrap = ObjectWrap::Unwrap<
      RemoteControlWrapper>(args.Holder());
  RemoteControlDevice* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_level_control_command level;
  artik_zigbee_endpoint endpoint;

  log_dbg("level_control_request");

  if (!args[0]->IsObject() || !args[1]->IsObject()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  if (convert_jsobject_levelcontrol(isolate, args[1]->ToObject(), &level)
      < 0) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  convert_jsobject_endpoint(isolate, args[0]->ToObject(), &endpoint);

  ret = obj->level_control_request(&endpoint, &level);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * request_reporting({
 *   node_id: N,
 *   endpoint_id: N,
 *   server_cluster: [...],
 *   client_cluster: [...]
 * }, 'measured_illuminance', 4, 10, 1.0)
 */
void RemoteControlWrapper::request_reporting(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  RemoteControlWrapper* wrap = ObjectWrap::Unwrap<
      RemoteControlWrapper>(args.Holder());
  RemoteControlDevice* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_endpoint endpoint;
  artik_zigbee_reporting_type report_type;
  float change_threshold;

  log_dbg("request_reporting");

  if (!args[0]->IsObject() || !args[1]->IsString()
      || !args[2]->IsInt32() || !args[3]->IsInt32()
      || !args[4]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[1]->ToString());
  char *command = *param0;

  if (!g_strcmp0(command, "thermostat_temperature")) {
    report_type = ARTIK_ZIGBEE_REPORTING_THERMOSTAT_TEMPERATURE;
  } else if (!g_strcmp0(command, "occupancy_sensing")) {
    report_type = ARTIK_ZIGBEE_REPORTING_OCCUPANCY_SENSING;
  } else if (!g_strcmp0(command, "measured_illuminance")) {
    report_type = ARTIK_ZIGBEE_REPORTING_MEASURED_ILLUMINANCE;
  } else if (!g_strcmp0(command, "measured_temperature")) {
    report_type = ARTIK_ZIGBEE_REPORTING_MEASURED_TEMPERATURE;
  } else {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  convert_jsobject_endpoint(isolate, args[0]->ToObject(), &endpoint);

  change_threshold = args[4]->NumberValue();
  log_dbg("%d, min:%d, max:%d, thresold:%f", report_type,
      args[2]->Int32Value(), args[3]->Int32Value(), change_threshold);

  ret = obj->request_reporting(&endpoint, report_type, args[2]->Int32Value(),
      args[3]->Int32Value(), change_threshold);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * stop_reporting({
 *   node_id: N,
 *   endpoint_id: N,
 *   server_cluster: [...],
 *   client_cluster: [...]
 * }, 'measured_illuminance')
 */
void RemoteControlWrapper::stop_reporting(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  RemoteControlWrapper* wrap = ObjectWrap::Unwrap<
      RemoteControlWrapper>(args.Holder());
  RemoteControlDevice* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_endpoint endpoint;
  artik_zigbee_reporting_type report_type;

  log_dbg("stop_reporting");

  if (!args[0]->IsObject() || !args[1]->IsString()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[1]->ToString());
  char *command = *param0;

  if (!g_strcmp0(command, "thermostat_temperature")) {
    report_type = ARTIK_ZIGBEE_REPORTING_THERMOSTAT_TEMPERATURE;
  } else if (!g_strcmp0(command, "occupancy_sensing")) {
    report_type = ARTIK_ZIGBEE_REPORTING_OCCUPANCY_SENSING;
  } else if (!g_strcmp0(command, "measured_illuminance")) {
    report_type = ARTIK_ZIGBEE_REPORTING_MEASURED_ILLUMINANCE;
  } else if (!g_strcmp0(command, "measured temperature")) {
    report_type = ARTIK_ZIGBEE_REPORTING_MEASURED_TEMPERATURE;
  } else {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  convert_jsobject_endpoint(isolate, args[0]->ToObject(), &endpoint);

  log_dbg("%d", report_type);

  ret = obj->stop_reporting(&endpoint, report_type);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * reset_to_factory_default({
 *   node_id: N,
 *   endpoint_id: N,
 *   server_cluster: [...],
 *   client_cluster: [...]
 * })
 */
void RemoteControlWrapper::reset_to_factory_default(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  RemoteControlWrapper* wrap = ObjectWrap::Unwrap<RemoteControlWrapper>(
      args.Holder());
  RemoteControlDevice* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_endpoint endpoint;

  log_dbg("reset_to_factory_default");

  if (!args[0]->IsObject()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  convert_jsobject_endpoint(isolate, args[0]->ToObject(), &endpoint);

  ret = obj->reset_to_factory_default(&endpoint);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * identify_request({
 *   node_id: N,
 *   endpoint_id: N,
 *   server_cluster: [...],
 *   client_cluster: [...]
 * }, 10)
 */
void RemoteControlWrapper::identify_request(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  RemoteControlWrapper* wrap = ObjectWrap::Unwrap<RemoteControlWrapper>(
      args.Holder());
  RemoteControlDevice* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_endpoint endpoint;

  log_dbg("identify_request");

  if (!args[0]->IsObject() || !args[1]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  convert_jsobject_endpoint(isolate, args[0]->ToObject(), &endpoint);

  ret = obj->identify_request(&endpoint, args[1]->Int32Value());
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * var remaining_time = identify_get_remaining_time({
 *   node_id: N,
 *   endpoint_id: N,
 *   server_cluster: [...],
 *   client_cluster: [...]
 * })
 */
void RemoteControlWrapper::identify_get_remaining_time(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  RemoteControlWrapper* wrap = ObjectWrap::Unwrap<RemoteControlWrapper>(
      args.Holder());
  RemoteControlDevice* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_endpoint endpoint;
  int remaining_time = 0;

  log_dbg("identify_get_remaining_time");

  if (!args[0]->IsObject()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  convert_jsobject_endpoint(isolate, args[0]->ToObject(), &endpoint);

  ret = obj->identify_get_remaining_time(&endpoint, &remaining_time);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Int32::New(isolate, remaining_time));
}

/**
 * ezmode_commissioning_target_start()
 */
void RemoteControlWrapper::ezmode_commissioning_target_start(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  RemoteControlWrapper* wrap = ObjectWrap::Unwrap<RemoteControlWrapper>(
      args.Holder());
  RemoteControlDevice* obj = wrap->getObj();
  artik_error ret;

  log_dbg("ezmode_commissioning_target_start");

  ret = obj->ezmode_commissioning_target_start();
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * ezmode_commissioning_target_stop()
 */
void RemoteControlWrapper::ezmode_commissioning_target_stop(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  RemoteControlWrapper* wrap = ObjectWrap::Unwrap<RemoteControlWrapper>(
      args.Holder());
  RemoteControlDevice* obj = wrap->getObj();
  artik_error ret;

  log_dbg("ezmode_commissioning_target_stop");

  ret = obj->ezmode_commissioning_target_stop();
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

}  // namespace artik
