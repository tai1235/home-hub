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

#include "zigbee/zigbee.h"

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <artik_log.h>

#include <list>

#include "zigbee/zigbee_util.h"
#include "zigbee/zigbee_device.h"

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

Persistent<Function> ZigbeeWrapper::constructor;

ZigbeeWrapper::ZigbeeWrapper() :
    m_init_cb(0) {
  m_zb = new Zigbee();
  m_loop = GlibLoop::Instance();
  m_loop->attach();
}

ZigbeeWrapper::~ZigbeeWrapper() {
  delete m_zb;
  m_loop->detach();
}

void ZigbeeWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("Init");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "zigbee"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "initialize", initialize);
  NODE_SET_PROTOTYPE_METHOD(tpl, "network_start", network_start);
  NODE_SET_PROTOTYPE_METHOD(tpl, "network_form", network_form);
  NODE_SET_PROTOTYPE_METHOD(tpl, "network_form_manually",
                            network_form_manually);
  NODE_SET_PROTOTYPE_METHOD(tpl, "network_permitjoin", network_permitjoin);
  NODE_SET_PROTOTYPE_METHOD(tpl, "network_leave", network_leave);
  NODE_SET_PROTOTYPE_METHOD(tpl, "network_join", network_join);
  NODE_SET_PROTOTYPE_METHOD(tpl, "network_find", network_find);
  NODE_SET_PROTOTYPE_METHOD(tpl, "network_request_my_network_status",
                            network_request_my_network_status);
  NODE_SET_PROTOTYPE_METHOD(tpl, "device_request_my_node_type",
                            device_request_my_node_type);
  NODE_SET_PROTOTYPE_METHOD(tpl, "device_find_by_cluster",
                            device_find_by_cluster);
  NODE_SET_PROTOTYPE_METHOD(tpl, "reset_local", reset_local);
  NODE_SET_PROTOTYPE_METHOD(tpl, "network_stop_scan", network_stop_scan);
  NODE_SET_PROTOTYPE_METHOD(tpl, "network_join_manually",
                            network_join_manually);
  NODE_SET_PROTOTYPE_METHOD(tpl, "device_discover", device_discover);
  NODE_SET_PROTOTYPE_METHOD(tpl, "set_discover_cycle_time",
                            set_discover_cycle_time);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_discovered_device_list",
                            get_discovered_device_list);
  NODE_SET_PROTOTYPE_METHOD(tpl, "raw_request", raw_request);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_local_device_list",
                            get_local_device_list);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "zigbee"), tpl->GetFunction());

  OnOffLightWrapper::Init(exports);
  OnOffSwitchWrapper::Init(exports);
  LevelControlSwitchWrapper::Init(exports);
  DimmableLightWrapper::Init(exports);
  LightSensorWrapper::Init(exports);
  RemoteControlWrapper::Init(exports);
}

void ZigbeeWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.IsConstructCall()) {
    ZigbeeWrapper* obj = new ZigbeeWrapper();
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
 * initialize(function() {})
 */
void ZigbeeWrapper::initialize(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(args.Holder());
  Zigbee* obj = wrap->getObj();

  log_dbg("initialize");

  if (!args[0]->IsFunction()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  wrap->m_init_cb = new v8::Persistent<v8::Function>();
  wrap->m_init_cb->Reset(isolate, Local<Function>::Cast(args[0]));

  obj->initialize(zb_callback, reinterpret_cast<void*>(wrap));

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * var state = network_start()
 */
void ZigbeeWrapper::network_start(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(args.Holder());
  Zigbee* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_network_state state;

  log_dbg("network_start");

  ret = obj->network_start(&state);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  log_dbg("- state(%d)", state);

  switch (state) {
  case ARTIK_ZIGBEE_NO_NETWORK:
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, "no_network"));
    break;
  case ARTIK_ZIGBEE_JOINING_NETWORK:
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, "joining_network"));
    break;
  case ARTIK_ZIGBEE_JOINED_NETWORK:
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, "joined_network"));
    break;
  case ARTIK_ZIGBEE_JOINED_NETWORK_NO_PARENT:
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, "joined_network_no_parent"));
    break;
  case ARTIK_ZIGBEE_LEAVING_NETWORK:
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, "leaving_network"));
    break;
  default:
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, "unknown"));
    log_err("unknown state(%d)", state);
    break;
  }
}

/**
 * network_form()
 */
void ZigbeeWrapper::network_form(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(args.Holder());
  Zigbee* obj = wrap->getObj();

  log_dbg("network_form");

  obj->network_form();

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * network_form_manually(channel, tx_power, pan_id)
 */
void ZigbeeWrapper::network_form_manually(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(args.Holder());
  Zigbee* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_network_info network_info;

  log_dbg("network_form_manually");

  if (!args[0]->IsInt32() || !args[1]->IsInt32()
      || !args[2]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  network_info.channel = args[0]->Int32Value();
  network_info.tx_power = (artik_zigbee_tx_power) args[1]->Int32Value();
  network_info.pan_id = args[2]->Int32Value();
  log_dbg("- channel(%d), tx_power(%d), pan_id(%d)",
      network_info.channel, network_info.tx_power, network_info.pan_id);

  ret = obj->network_form_manually(&network_info);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * network_permitjoin(duration_sec)
 */
void ZigbeeWrapper::network_permitjoin(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(args.Holder());
  Zigbee* obj = wrap->getObj();
  artik_error ret;

  log_dbg("network_permitjoin");

  if (!args[0]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  int duration_sec = args[0]->Int32Value();
  log_dbg("- duration(%d)", duration_sec);

  ret = obj->network_permitjoin(duration_sec);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * network_leave()
 */
void ZigbeeWrapper::network_leave(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(args.Holder());
  Zigbee* obj = wrap->getObj();
  artik_error ret;

  log_dbg("network_leave");

  ret = obj->network_leave();
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * network_join()
 */
void ZigbeeWrapper::network_join(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(args.Holder());
  Zigbee* obj = wrap->getObj();

  log_dbg("network_join");

  obj->network_join();

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * network_find()
 */
void ZigbeeWrapper::network_find(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(args.Holder());
  Zigbee* obj = wrap->getObj();

  log_dbg("network_find");

  obj->network_find();

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * var status = network_request_my_network_status()
 */
void ZigbeeWrapper::network_request_my_network_status(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(args.Holder());
  Zigbee* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_network_state status;

  log_dbg("network_request_my_network_status");

  ret = obj->network_request_my_network_status(&status);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  switch (status) {
  case ARTIK_ZIGBEE_NO_NETWORK:
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, "no_network"));
    break;
  case ARTIK_ZIGBEE_JOINING_NETWORK:
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, "joining_network"));
    break;
  case ARTIK_ZIGBEE_JOINED_NETWORK:
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, "joined_network"));
    break;
  case ARTIK_ZIGBEE_JOINED_NETWORK_NO_PARENT:
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, "joined_network_no_parent"));
    break;
  case ARTIK_ZIGBEE_LEAVING_NETWORK:
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, "leaving_network"));
    break;
  default:
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, "leaving_network"));
    log_err("unknown status(%d)", status);
    break;
  }
}

/**
 * var type = device_request_my_node_type()
 */
void ZigbeeWrapper::device_request_my_node_type(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(args.Holder());
  Zigbee* obj = wrap->getObj();
  artik_error ret = S_OK;
  artik_zigbee_node_type type;

  log_dbg("device_request_my_node_type");

  ret = obj->device_request_my_node_type(&type);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  switch (type) {
  case ARTIK_ZIGBEE_UNKNOWN_DEVICE:
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, "unknown_device"));
    break;
  case ARTIK_ZIGBEE_COORDINATOR:
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, "coordinator"));
    break;
  case ARTIK_ZIGBEE_ROUTER:
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, "router"));
    break;
  case ARTIK_ZIGBEE_END_DEVICE:
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, "end_device"));
    break;
  case ARTIK_ZIGBEE_SLEEPY_END_DEVICE:
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, "sleepy_end_device"));
    break;
  default:
    args.GetReturnValue().Set(
        String::NewFromUtf8(isolate, "unknown"));
    log_err("unknown type (%d)", type);
    break;
  }
}

/**
 * var endpoint_list = device_find_by_cluster(cluster_id)
 * console.log(endpoint_list)
 * [{
 *   endpoint_id: N,
 *   device_id: N,
 *   server_cluster: [ 1, 2, 3,-1,-1,-1,-1,-1,-1],
 *   client_cluster: [ 1, 2,-1,-1,-1,-1,-1,-1,-1]
 * }, {
 *   ...
 * }]
 */
void ZigbeeWrapper::device_find_by_cluster(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(args.Holder());
  Zigbee* obj = wrap->getObj();
  artik_zigbee_endpoint_list endpointList;
  char *json_res = NULL;

  log_dbg("device_find_by_cluster");

  if (!args[0]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  int cluster_id = args[0]->Int32Value();

  obj->device_find_by_cluster(&endpointList, cluster_id, 1);

  json_res = convert_endpointlist_to_json(&endpointList);

  args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, json_res));

  if (json_res)
    free(json_res);
}

/**
 * reset_local()
 */
void ZigbeeWrapper::reset_local(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(args.Holder());
  Zigbee* obj = wrap->getObj();
  artik_error ret;

  log_dbg("reset_local");

  ret = obj->reset_local();
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * network_stop_scan()
 */
void ZigbeeWrapper::network_stop_scan(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(args.Holder());
  Zigbee* obj = wrap->getObj();
  artik_error ret;

  log_dbg("network_stop_scan");

  ret = obj->network_stop_scan();
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * network_join_manually(channel, tx_power, pan_id)
 */
void ZigbeeWrapper::network_join_manually(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(args.Holder());
  Zigbee* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_network_info network_info;

  log_dbg("network_join_manually");

  if (!args[0]->IsInt32() || !args[1]->IsInt32()
      || !args[2]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  network_info.channel = args[0]->Int32Value();
  network_info.tx_power = (artik_zigbee_tx_power) args[1]->Int32Value();
  network_info.pan_id = args[2]->Int32Value();
  log_dbg("- channel(%d), tx_power(%d), pan_id(%d)",
      network_info.channel, network_info.tx_power, network_info.pan_id);

  ret = obj->network_join_manually(&network_info);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * device_discover()
 */
void ZigbeeWrapper::device_discover(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(args.Holder());
  Zigbee* obj = wrap->getObj();
  artik_error ret;

  log_dbg("device_discover");

  ret = obj->device_discover();
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * device_discover(time_minutes)
 */
void ZigbeeWrapper::set_discover_cycle_time(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(args.Holder());
  Zigbee* obj = wrap->getObj();
  artik_error ret;

  log_dbg("set_discover_cycle_time");

  if (!args[0]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  int time_minutes = args[0]->Int32Value();
  log_dbg("- time_minutes(%d)", time_minutes);

  ret = obj->set_discover_cycle_time(time_minutes);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  args.GetReturnValue().Set(Undefined(isolate));
}

/**
 * var dinfo = get_discovered_device_list()
 * console.log(dinfo)
 * [{
 *   eui64: 'XXXXXXXXXXXXXXXX',
 *   device_id: N,
 *   endpoints: [{
 *     endpoint_id: N,
 *     device_id: N,
 *     server_cluster: [ 1, 2, 3,-1,-1,-1,-1,-1,-1],
 *     client_cluster: [ 1, 2,-1,-1,-1,-1,-1,-1,-1]
 *   }, {
 *     ...
 *   }]
 * }, {
 *   ...
 * }]
 */
void ZigbeeWrapper::get_discovered_device_list(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(args.Holder());
  Zigbee* obj = wrap->getObj();
  artik_error ret;
  artik_zigbee_device_info device_info;
  char *json_res;

  log_dbg("get_discovered_device_list");

  ret = obj->get_discovered_device_list(&device_info);
  if (ret != S_OK) {
    throw_error(isolate, ret);
    return;
  }

  json_res = convert_device_info(&device_info);

  args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, json_res));

  if (json_res)
    free(json_res);
}

/**
 * raw_request(command)
 */
void ZigbeeWrapper::raw_request(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(args.Holder());
  Zigbee* obj = wrap->getObj();

  log_dbg("raw_request");

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[0]->ToString());

  obj->raw_request(*param0);

  args.GetReturnValue().Set(Undefined(isolate));
}

static char* convert_device(ZigbeeDevice *dev) {
  const char *tpl = "{ "
      "\"device_id\": %d, "
      "\"profile_id\": %d, "
      "\"handle\": %d "
      " }";

  return g_strdup_printf(tpl, dev->get_device_id(), dev->get_profile_id(),
      (intptr_t)(dev->get_handle()));
}

/**
 * var dlist = get_device_list()
 * console.log(dlist)
 * [{
 *   device_id: N,
 *   profile_id: N,
 *   handle: N
 * }, {
 *   ...
 * }]
 */
void ZigbeeWrapper::get_local_device_list(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  ZigbeeWrapper* wrap = ObjectWrap::Unwrap<ZigbeeWrapper>(args.Holder());
  Zigbee* obj = wrap->getObj();
  std::list<ZigbeeDevice*> result;
  std::list<artik::ZigbeeDevice*>::iterator iter;
  gchar **str_array;
  gchar *tmp;
  gchar *json_res;
  int i = 0;

  log_dbg("get_local_device_list");

  result = obj->get_local_device_list();

  str_array = g_new0(gchar *, result.size() + 1);
  for (iter = result.begin(); iter != result.end(); iter++, i++) {
    str_array[i] = convert_device(*iter);
  }

  tmp = g_strjoinv(",", str_array);
  g_strfreev(str_array);

  json_res = g_strdup_printf("[%s]", tmp);
  g_free(tmp);

  args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, json_res));

  if (json_res)
    free(json_res);
}

}  // namespace artik
