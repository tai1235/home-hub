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

#include "bluetooth/bluetooth.h"

#include <artik_log.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <node_buffer.h>
#include <utils.h>

#include <vector>
#include <string>

#include "bluetooth/agent.h"
#include "bluetooth/avrcp.h"
#include "bluetooth/a2dp.h"
#include "bluetooth/ftp.h"
#include "bluetooth/gatt_client.h"
#include "bluetooth/gatt_server.h"
#include "bluetooth/pan.h"
#include "bluetooth/spp.h"

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
using v8::Uint32;
using v8::Boolean;
using v8::Null;
using v8::Context;

#define JSON_MAX_ENC_LEN  20

Persistent<Function> BluetoothWrapper::constructor;

std::array<const char*, 3> BluetoothWrapper::bt_scan_types = {
  "auto",
  "bredr",
  "le"
};

std::array<const char*, 3> BluetoothWrapper::bt_device_types = {
  "paired",
  "connected",
  "all"
};

std::array<const char*, 6> BluetoothWrapper::bt_device_properties = {
  "Address",
  "Name",
  "Icon",
  "Alias",
  "Modalias"
};

std::unordered_map<int, const char*> BluetoothWrapper::bt_major_device_class = {
  {BT_MAJOR_DEVICE_CLASS_MISC, "misc"},
  {BT_MAJOR_DEVICE_CLASS_COMPUTER, "computer"},
  {BT_MAJOR_DEVICE_CLASS_PHONE, "phone"},
  {BT_MAJOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT, "nap"},
  {BT_MAJOR_DEVICE_CLASS_AUDIO_VIDEO, "media"},
  {BT_MAJOR_DEVICE_CLASS_PERIPHERAL, "peripheral"},
  {BT_MAJOR_DEVICE_CLASS_IMAGING, "imaging"},
  {BT_MAJOR_DEVICE_CLASS_WEARABLE, "wearable"},
  {BT_MAJOR_DEVICE_CLASS_TOY, "toy"},
  {BT_MAJOR_DEVICE_CLASS_HEALTH, "health"},
  {BT_MAJOR_DEVICE_CLASS_UNCATEGORIZED, "uncategorized"}
};

std::unordered_map<int, const char*> BluetoothWrapper::bt_minor_device_class = {
  {BT_MINOR_DEVICE_CLASS_COMPUTER_UNCATEGORIZED, "computer/uncategorized"},
  {BT_MINOR_DEVICE_CLASS_COMPUTER_DESKTOP_WORKSTATION, "computer/desktop"},
  {BT_MINOR_DEVICE_CLASS_COMPUTER_SERVER_CLASS, "computer/server"},
  {BT_MINOR_DEVICE_CLASS_COMPUTER_LAPTOP, "computer/laptop"},
  {BT_MINOR_DEVICE_CLASS_COMPUTER_HANDHELD_PC_OR_PDA, "computer/handheld"},
  {BT_MINOR_DEVICE_CLASS_COMPUTER_PALM_SIZED_PC_OR_PDA, "computer/palm"},
  {BT_MINOR_DEVICE_CLASS_COMPUTER_WEARABLE_COMPUTER, "computer/wearable"},
  {BT_MINOR_DEVICE_CLASS_PHONE_UNCATEGORIZED, "phone/uncategorized"},
  {BT_MINOR_DEVICE_CLASS_PHONE_CELLULAR, "phone/cellular"},
  {BT_MINOR_DEVICE_CLASS_PHONE_CORDLESS, "phone/cordless"},
  {BT_MINOR_DEVICE_CLASS_PHONE_SMART_PHONE, "phone/smartphone"},
  {BT_MINOR_DEVICE_CLASS_PHONE_WIRED_MODEM_OR_VOICE_GATEWAY, "phone/gateway"},
  {BT_MINOR_DEVICE_CLASS_PHONE_COMMON_ISDN_ACCESS, "phone/isdn"},
  {BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_FULLY_AVAILABLE, "nap/full"},
  {BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_1_TO_17_PERCENT_UTILIZED,
      "nap/1-17"},
  {BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_17_TO_33_PERCENT_UTILIZED,
      "nap/17-33"},
  {BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_33_TO_50_PERCENT_UTILIZED,
      "nap/33-50"},
  {BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_50_to_67_PERCENT_UTILIZED,
      "nap/50-67"},
  {BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_67_TO_83_PERCENT_UTILIZED,
      "nap/67-83"},
  {BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_83_TO_99_PERCENT_UTILIZED,
      "nap/83-99"},
  {BT_MINOR_DEVICE_CLASS_LAN_NETWORK_ACCESS_POINT_NO_SERVICE_AVAILABLE,
      "nap/noservice"},
  {BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_UNCATEGORIZED, "media/uncategorized"},
  {BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_WEARABLE_HEADSET, "media/wearable"},
  {BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_HANDS_FREE, "media/handsfree"},
  {BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_MICROPHONE, "media/microphone"},
  {BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_LOUDSPEAKER, "media/loudspeaker"},
  {BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_HEADPHONES, "media/headphones"},
  {BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_PORTABLE_AUDIO, "media/audio/portable"},
  {BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_CAR_AUDIO, "media/audio/car"},
  {BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_SET_TOP_BOX, "media/set-top"},
  {BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_HIFI_AUDIO_DEVICE, "media/audio/hifi"},
  {BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_VCR, "media/vcr"},
  {BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_VIDEO_CAMERA, "media/camera"},
  {BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_CAMCORDER, "media/camorder"},
  {BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_VIDEO_MONITOR, "media/monitor"},
  {BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_VIDEO_DISPLAY_LOUDSPEAKER,
      "media/display/loudspeaker"},
  {BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_VIDEO_CONFERENCING, "media/conferencing"},
  {BT_MINOR_DEVICE_CLASS_AUDIO_VIDEO_GAMING_TOY, "media/gaming"},
  {BT_MINOR_DEVICE_CLASS_PERIPHERA_UNCATEGORIZED, "peripheral/uncategorized"},
  {BT_MINOR_DEVICE_CLASS_PERIPHERAL_KEY_BOARD, "peripheral/keyboard"},
  {BT_MINOR_DEVICE_CLASS_PERIPHERAL_POINTING_DEVICE, "peripheral/pointing"},
  {BT_MINOR_DEVICE_CLASS_PERIPHERAL_COMBO_KEYBOARD_POINTING_DEVICE,
      "peripheral/keyboard/pointing"},
  {BT_MINOR_DEVICE_CLASS_PERIPHERAL_JOYSTICK, "peripheral/joystick"},
  {BT_MINOR_DEVICE_CLASS_PERIPHERAL_GAME_PAD, "peripheral/gamepad"},
  {BT_MINOR_DEVICE_CLASS_PERIPHERAL_REMOTE_CONTROL, "peripheral/remote"},
  {BT_MINOR_DEVICE_CLASS_PERIPHERAL_SENSING_DEVICE, "peripheral/sensing"},
  {BT_MINOR_DEVICE_CLASS_PERIPHERAL_DIGITIZER_TABLET, "peripheral/digitizer"},
  {BT_MINOR_DEVICE_CLASS_PERIPHERAL_CARD_READER, "peripheral/cardreader"},
  {BT_MINOR_DEVICE_CLASS_PERIPHERAL_DIGITAL_PEN, "peripheral/pen"},
  {BT_MINOR_DEVICE_CLASS_PERIPHERAL_HANDHELD_SCANNER, "peripheral/scanner"},
  {BT_MINOR_DEVICE_CLASS_PERIPHERAL_HANDHELD_GESTURAL_INPUT_DEVICE,
      "peripheral/gestural"},
  {BT_MINOR_DEVICE_CLASS_IMAGING_DISPLAY, "imaging/display"},
  {BT_MINOR_DEVICE_CLASS_IMAGING_CAMERA, "imaging/camera"},
  {BT_MINOR_DEVICE_CLASS_IMAGING_SCANNER, "imaging/scanner"},
  {BT_MINOR_DEVICE_CLASS_IMAGING_PRINTER, "imaging/printer"},
  {BT_MINOR_DEVICE_CLASS_WEARABLE_WRIST_WATCH, "wearable/watch"},
  {BT_MINOR_DEVICE_CLASS_WEARABLE_PAGER, "wearable/pager"},
  {BT_MINOR_DEVICE_CLASS_WEARABLE_JACKET, "wearable/jacket"},
  {BT_MINOR_DEVICE_CLASS_WEARABLE_HELMET, "wearable/helmet"},
  {BT_MINOR_DEVICE_CLASS_WEARABLE_GLASSES, "wearable/glasses"},
  {BT_MINOR_DEVICE_CLASS_TOY_ROBOT, "toy/robot"},
  {BT_MINOR_DEVICE_CLASS_TOY_VEHICLE, "toy/vehicle"},
  {BT_MINOR_DEVICE_CLASS_TOY_DOLL_ACTION, "toy/doll"},
  {BT_MINOR_DEVICE_CLASS_TOY_CONTROLLER, "toy/controller"},
  {BT_MINOR_DEVICE_CLASS_TOY_GAME, "toy/game"},
  {BT_MINOR_DEVICE_CLASS_HEALTH_BLOOD_PRESSURE_MONITOR,
      "health/monitor/bloodpresure"},
  {BT_MINOR_DEVICE_CLASS_HEALTH_THERMOMETER, "health/thermometer"},
  {BT_MINOR_DEVICE_CLASS_HEALTH_WEIGHING_SCALE, "health/weighing"},
  {BT_MINOR_DEVICE_CLASS_HEALTH_GLUCOSE_METER, "health/glucose"},
  {BT_MINOR_DEVICE_CLASS_HEALTH_PULSE_OXIMETER, "health/oximeter"},
  {BT_MINOR_DEVICE_CLASS_HEALTH_HEART_PULSE_RATE_MONITOR,
      "health/monitor/pluserate"},
  {BT_MINOR_DEVICE_CLASS_HEALTH_DATA_DISPLAY, "health/display"},
  {BT_MINOR_DEVICE_CLASS_HEALTH_STEP_COUNTER, "health/stepcounter"},
  {BT_MINOR_DEVICE_CLASS_HEALTH_BODY_COMPOSITION_ANALYZER,
      "health/composition"},
  {BT_MINOR_DEVICE_CLASS_HEALTH_PEAK_FLOW_MONITOR, "health/monitor/pefr"},
  {BT_MINOR_DEVICE_CLASS_HEALTH_MEDICATION_MONITOR,
      "health/monitor/medication"},
  {BT_MINOR_DEVICE_CLASS_HEALTH_KNEE_PROSTHESIS, "health/prosthesis/knee"},
  {BT_MINOR_DEVICE_CLASS_HEALTH_ANKLE_PROSTHESIS, "health/prosthesis/ankle"}
};
std::unordered_map<int, const char*>
    BluetoothWrapper::bt_device_service_class = {
  {BT_SERVICE_CLASS_LIMITED_DISCOVERABLE_MODE, "limited_discoverable_mode"},
  {BT_SERVICE_CLASS_POSITIONING, "positioning"},
  {BT_SERVICE_CLASS_NETWORKING, "networking"},
  {BT_SERVICE_CLASS_RENDERING, "rendering"},
  {BT_SERVICE_CLASS_CAPTURING, "capturing"},
  {BT_SERVICE_CLASS_OBJECT_TRANSFER, "transfer"},
  {BT_SERVICE_CLASS_AUDIO, "audio"},
  {BT_SERVICE_CLASS_TELEPHONY, "telephony"},
  {BT_SERVICE_CLASS_INFORMATION, "information"},
};

static char* convert_devices_to_json(artik_bt_device* devs, int num_devs) {
  char* json;
  int i = 0, j = 0;
  int num_uuids = 0;
  int uuid_len = 0;
  int max_entry_len, max_uuids_len, max_json_len;
  char header[] = "\n[\n";
  char dev_entry[] = "\t{\n"
             "\t\t\"address\": \"%s\",\n"
             "\t\t\"name\": \"%s\",\n"
             "\t\t\"rssi\": %d,\n"
             "\t\t\"bonded\": %s,\n"
             "\t\t\"connected\": %s,\n"
             "\t\t\"authorized\": %s,\n"
             "\t\t\"class\":{\n"
             "\t\t\t\"major\": %d,\n"
                 "\t\t\t\"minor\": %d,\n"
                 "\t\t\t\"service\": %d\n"
             "\t\t},\n"
                 "\t\t\"uuids\":[\n"
             "%s"
             "\t\t]\n"
             "\t},\n";
  char uuid_entry[] = "\t\t\t{\n"
            "\t\t\t\t\"name\": \"%s\",\n"
            "\t\t\t\t\"uuid\": \"%s\"\n"
            "\t\t\t},\n";
  char footer[] = "]\n";

  log_dbg("");

  // Compute max length for the JSON string and do the allocation
  max_entry_len = strlen(dev_entry)
          + MAX_BT_ADDR_LEN   // address
          + MAX_BT_NAME_LEN   // name
          + 6                 // bonded
          + 6                 // connected
          + 6                 // authorized
          + 11                // major
          + 11                // minor
          + 11;               // service

  // Count total number of uuids to store in json
  for (i = 0; i < num_devs; i++)
    num_uuids += devs[i].uuid_length;

  uuid_len = strlen(uuid_entry) + MAX_BT_UUID_LEN + MAX_BT_UUID_LEN;
  max_uuids_len = uuid_len * num_uuids;

  max_json_len = (max_entry_len*num_devs) + max_uuids_len + strlen(header) +
      strlen(footer) + 1;
  json = reinterpret_cast<char*>(malloc(max_json_len));
  if (!json)
    return json;

  // Start building the JSON string
  memset(json, 0, max_json_len);
  strncat(json, header, strlen(header));

  for (i = 0; i < num_devs; i++) {
    int uuids_len = devs[i].uuid_length * uuid_len;
    char* uuids = NULL;
    char* entry = reinterpret_cast<char*>(malloc(max_entry_len + uuids_len));
    if (!entry)
      break;

    // Fill up UUID list
    if (devs[i].uuid_length > 0) {
      uuids = reinterpret_cast<char*>(malloc(uuids_len));
      if (!uuids) {
        break;
      }

      memset(uuids, 0, uuids_len);

      for (j = 0; j < devs[i].uuid_length; j++) {
        char* uuid = reinterpret_cast<char*>(malloc(uuid_len));
        if (!uuid)
          break;

        snprintf(uuid, uuid_len, uuid_entry, devs[i].uuid_list[j].uuid_name,
            devs[i].uuid_list[j].uuid);
        strncat(uuids, uuid, uuids_len);
        free(uuid);
      }

      // Remove last comma
      uuids[strlen(uuids)-2] = '\n';
      uuids[strlen(uuids)-1] = '\0';
    }

    snprintf(entry, max_entry_len + uuids_len, dev_entry,
        devs[i].remote_address,
        devs[i].remote_name,
        devs[i].rssi,
        devs[i].is_bonded ? "true" : "false",
        devs[i].is_connected ? "true" : "false",
        devs[i].is_authorized ? "true" : "false",
        devs[i].cod.major,
        devs[i].cod.minor,
        devs[i].cod.service_class,
        uuids ? uuids : " ");

    strncat(json, entry, max_entry_len + uuids_len);

    free(entry);
    if ((devs[i].uuid_length > 0) && uuids)
      free(uuids);
  }

  // Remove last comma
  json[strlen(json)-2] = '\n';
  json[strlen(json)-1] = '\0';

  strncat(json, footer, strlen(footer));

  return json;
}

BluetoothWrapper::BluetoothWrapper() {
  m_bt = new Bluetooth();
  m_loop = GlibLoop::Instance();
  m_loop->attach();
}

BluetoothWrapper::~BluetoothWrapper() {
  delete m_bt;
  m_loop->detach();
}

void BluetoothWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "bluetooth"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "start_scan", start_scan);
  NODE_SET_PROTOTYPE_METHOD(tpl, "stop_scan", stop_scan);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_devices", get_devices);
  NODE_SET_PROTOTYPE_METHOD(tpl, "start_bond", start_bond);
  NODE_SET_PROTOTYPE_METHOD(tpl, "stop_bond", stop_bond);
  NODE_SET_PROTOTYPE_METHOD(tpl, "connect", connect);
  NODE_SET_PROTOTYPE_METHOD(tpl, "disconnect", disconnect);
  NODE_SET_PROTOTYPE_METHOD(tpl, "remove_unpaired_devices",
      remove_unpaired_devices);
  NODE_SET_PROTOTYPE_METHOD(tpl, "remove_device", remove_device);
  NODE_SET_PROTOTYPE_METHOD(tpl, "set_scan_filter", set_scan_filter);
  NODE_SET_PROTOTYPE_METHOD(tpl, "set_alias", set_alias);
  NODE_SET_PROTOTYPE_METHOD(tpl, "set_discoverable", set_discoverable);
  NODE_SET_PROTOTYPE_METHOD(tpl, "set_pairable", set_pairable);
  NODE_SET_PROTOTYPE_METHOD(tpl, "set_pairableTimeout", set_pairableTimeout);
  NODE_SET_PROTOTYPE_METHOD(tpl, "set_discoverableTimeout",
      set_discoverableTimeout);
  NODE_SET_PROTOTYPE_METHOD(tpl, "is_scanning", is_scanning);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_adapter_info", get_adapter_info);
  NODE_SET_PROTOTYPE_METHOD(tpl, "remove_devices", remove_devices);
  NODE_SET_PROTOTYPE_METHOD(tpl, "connect_profile", connect_profile);
  NODE_SET_PROTOTYPE_METHOD(tpl, "set_trust", set_trust);
  NODE_SET_PROTOTYPE_METHOD(tpl, "unset_trust", unset_trust);
  NODE_SET_PROTOTYPE_METHOD(tpl, "set_block", set_block);
  NODE_SET_PROTOTYPE_METHOD(tpl, "unset_block", unset_block);
  NODE_SET_PROTOTYPE_METHOD(tpl, "is_paired", is_paired);
  NODE_SET_PROTOTYPE_METHOD(tpl, "is_connected", is_connected);
  NODE_SET_PROTOTYPE_METHOD(tpl, "is_trusted", is_trusted);
  NODE_SET_PROTOTYPE_METHOD(tpl, "is_blocked", is_blocked);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "bluetooth"),
               tpl->GetFunction());

  AgentRequestWrapper::Init(exports);
  AgentWrapper::Init(exports);
  AvrcpWrapper::Init(exports);
  A2dpWrapper::Init(exports);
  PanWrapper::Init(exports);
  SppWrapper::Init(exports);
  SppSocketWrapper::Init(exports);
  FtpWrapper::Init(exports);
  GattServerWrapper::Init(exports);
  CharacteristicWrapper::Init(exports);
  DescriptorWrapper::Init(exports);
  ServiceWrapper::Init(exports);
  GattClientWrapper::Init(exports);
  RemoteDescriptorWrapper::Init(exports);
  RemoteCharacteristicWrapper::Init(exports);
  RemoteServiceWrapper::Init(exports);
}

void BluetoothWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  log_dbg("");

  if (args.IsConstructCall()) {
    BluetoothWrapper* obj = new BluetoothWrapper();
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

static void on_callback(artik_bt_event event, void *data, void *user_data) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope handleScope(isolate);
  BluetoothWrapper *wrap = reinterpret_cast<BluetoothWrapper*>(user_data);

  log_dbg("");

  switch (event) {
    case BT_EVENT_SCAN:
    {
      if (!wrap->getScanCb())
        return;

      char* json_res = convert_devices_to_json(
          reinterpret_cast<artik_bt_device*>(data), 1);

      Handle<Value> argv[] = {
        Null(isolate),
        Handle<Value>(String::NewFromUtf8(isolate,
            json_res ? json_res : "null data")),
      };

      if (json_res)
        free(json_res);

      Local<Function>::New(isolate, *wrap->getScanCb())->Call(
          isolate->GetCurrentContext()->Global(), 2, argv);
    }
    break;

    case BT_EVENT_BOND:
    {
      if (!wrap->getBondCb())
        return;

      bool paired = *reinterpret_cast<bool*>(data);

      Handle<Value> argv[] = {
        Null(isolate),
        Handle<Value>(String::NewFromUtf8(isolate,
            paired ? "paired" : "unpaired")),
      };

      Local<Function>::New(isolate, *wrap->getBondCb())->Call(
          isolate->GetCurrentContext()->Global(), 2, argv);
    }
    break;

    case BT_EVENT_CONNECT:
    {
      if (!wrap->getConnectCb())
        return;

      bool connected = *reinterpret_cast<bool*>(data);

      Handle<Value> argv[] = {
        Null(isolate),
        Handle<Value>(String::NewFromUtf8(isolate,
            connected ? "connected" : "disconnected")),
      };

      Local<Function>::New(isolate, *wrap->getConnectCb())->Call(
          isolate->GetCurrentContext()->Global(), 2, argv);
    }
    break;

    default:
    break;
  }
}

void BluetoothWrapper::start_scan(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  BluetoothWrapper* wrap = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder());
  Bluetooth* bt = wrap->getObj();
  artik_error ret = S_OK;

  log_dbg("");

  if (!args[0]->IsFunction()) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
      return;
  }

  wrap->m_scan_cb = new Persistent<Function>();
  wrap->m_scan_cb->Reset(isolate, Local<Function>::Cast(args[0]));
  bt->set_callback(BT_EVENT_SCAN, on_callback, reinterpret_cast<void*>(wrap));
  ret = bt->start_scan();
  if (ret != S_OK) {
    Handle<Value> argv[] = {
      Handle<Value>(
        String::NewFromUtf8(isolate,
                error_msg(ret))),
      Null(isolate)
    };
    Local<Function>::New(isolate,
      *wrap->getScanCb())->Call(
        isolate->GetCurrentContext()->Global(),
        2,
        argv);
  }
}

void BluetoothWrapper::stop_scan(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  BluetoothWrapper* wrap = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder());
  Bluetooth* bt = wrap->getObj();
  artik_error err = S_OK;

  log_dbg("");

  if (!wrap->m_scan_cb)
    return;

  err = bt->stop_scan();
  if (err != S_OK)
    goto exit;

  err = bt->unset_callback(BT_EVENT_SCAN);
  delete wrap->m_scan_cb;
  wrap->m_scan_cb = NULL;

exit:
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void BluetoothWrapper::get_devices(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  BluetoothWrapper* obj = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder());
  Bluetooth* bt = obj->getObj();
  int num_devs = 0;
  artik_bt_device *devices = NULL;
  artik_error err = S_OK;

  log_dbg("");

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
  }

  String::Utf8Value utf8Type(args[0]->ToString());
  auto type = to_artik_parameter<artik_bt_device_type>(bt_device_types,
    *utf8Type);
  if (!type) {
    std::string error = "Device type " + std::string(*utf8Type) +
        " is not supported";
    isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, error.c_str())));
    return;
  }

  err = bt->get_devices(type.value(), &devices, &num_devs);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }

  char* json_res = convert_devices_to_json(devices, num_devs);
  args.GetReturnValue().Set(String::NewFromUtf8(isolate,
      json_res ? json_res : "null data"));
  if (json_res)
    free(json_res);
  bt->free_devices(&devices, num_devs);
}

void BluetoothWrapper::start_bond(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  BluetoothWrapper* wrap = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder());
  Bluetooth* bt = wrap->getObj();
  artik_error ret = S_OK;

  log_dbg("");

  if (!args[0]->IsString() ||
      !args[1]->IsFunction()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  String::Utf8Value param0(args[0]->ToString());
  char* addr = *param0;

  wrap->m_bond_cb = new Persistent<Function>();
  wrap->m_bond_cb->Reset(isolate, Local<Function>::Cast(args[1]));
  bt->set_callback(BT_EVENT_BOND, on_callback, reinterpret_cast<void*>(wrap));

  ret = bt->start_bond(addr);

  if (ret != S_OK) {
    Handle<Value> argv[] = {
      Handle<Value>(
        String::NewFromUtf8(isolate,
                error_msg(ret))),
      Null(isolate)
    };
    Local<Function>::New(isolate,
      *wrap->getBondCb())->Call(
        isolate->GetCurrentContext()->Global(),
        2,
        argv);
  }
}

void BluetoothWrapper::stop_bond(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  BluetoothWrapper* wrap = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder());
  Bluetooth* bt = wrap->getObj();
  artik_error err = S_OK;

  log_dbg("");

  if (!wrap->m_bond_cb)
    return;

  if (!args[0]->IsString()) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
      return;
  }

  String::Utf8Value param0(args[0]->ToString());
  char* addr = *param0;

  err = bt->stop_bond(addr);
  if (err != S_OK)
    goto exit;

  err = bt->unset_callback(BT_EVENT_BOND);
  delete wrap->m_bond_cb;
  wrap->m_bond_cb = NULL;

exit:
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void BluetoothWrapper::connect(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  BluetoothWrapper* wrap = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder());
  Bluetooth* bt = wrap->getObj();
  artik_error ret = S_OK;

  log_dbg("");

  if (!args[0]->IsString() || !args[1]->IsFunction()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  String::Utf8Value param0(args[0]->ToString());
  char* addr = *param0;

  wrap->m_connect_cb = new Persistent<Function>();
  wrap->m_connect_cb->Reset(isolate, Local<Function>::Cast(args[1]));
  bt->set_callback(BT_EVENT_CONNECT, on_callback,
      reinterpret_cast<void*>(wrap));

  ret = bt->connect(addr);

  if (ret != S_OK) {
    Handle<Value> argv[] = {
      Handle<Value>(
        String::NewFromUtf8(isolate,
                error_msg(ret))),
      Null(isolate)
    };
    Local<Function>::New(isolate,
      *wrap->getConnectCb())->Call(
        isolate->GetCurrentContext()->Global(),
        2,
        argv);
  }
}

void BluetoothWrapper::disconnect(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  BluetoothWrapper* wrap = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder());
  Bluetooth* bt = wrap->getObj();
  artik_error err = S_OK;

  log_dbg("");

  if (!wrap->m_connect_cb)
    return;

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  String::Utf8Value param0(args[0]->ToString());
  char* addr = *param0;

  err = bt->disconnect(addr);
  if (err != S_OK)
    goto exit;

  err = bt->unset_callback(BT_EVENT_CONNECT);
  delete wrap->m_connect_cb;
  wrap->m_connect_cb = NULL;

exit:
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void BluetoothWrapper::remove_unpaired_devices(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder())->getObj();
  artik_error err = S_OK;

  log_dbg("");

  err = bt->remove_unpaired_devices();
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void BluetoothWrapper::remove_device(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder())->getObj();
  artik_error err = S_OK;

  log_dbg("");

  if (!args[0]->IsString()) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
      return;
  }

  String::Utf8Value param0(args[0]->ToString());
  char* addr = *param0;

  err = bt->remove_device(addr);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void BluetoothWrapper::set_scan_filter(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  if (!args[0]->IsObject()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong type argument for filter")));
    return;
  }

  Handle<Object> scan_filter = Handle<Object>::Cast(args[0]);
  Handle<Value> rssi = scan_filter->Get(String::NewFromUtf8(isolate, "rssi"));
  if (!rssi->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong type argument for rssi")));
    return;
  }

  Handle<Value> uuidsValue = scan_filter->Get(
      String::NewFromUtf8(isolate, "uuids"));
  if (!uuidsValue->IsArray()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong type argument for uuids")));
    return;
  }

  std::vector<artik_bt_uuid> uuids_vector;
  Handle<Array> uuids = Handle<Array>::Cast(uuidsValue);
  unsigned int uuids_length = uuids->Length();
  for (unsigned int i = 0; i < uuids_length; i++) {
    artik_bt_uuid uuid;
    String::Utf8Value utf8Value(uuids->Get(i)->ToString());
    uuid.uuid = strdup(*utf8Value);
    uuid.uuid_name = NULL;
    uuids_vector.push_back(uuid);
  }

  String::Utf8Value utf8Type(scan_filter->Get(
     String::NewFromUtf8(isolate, "type"))->ToString());
  auto type = to_artik_parameter<artik_bt_scan_type>(bt_scan_types, *utf8Type);
  if (!type) {
    std::string error = "Scan type " + std::string(*utf8Type) +
        " is not supported";
    isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, error.c_str())));
    return;
  }

  artik_bt_scan_filter filter {
    .uuid_list = uuids_vector.data(),
    .uuid_length = uuids_length,
    .rssi = static_cast<int16_t>(rssi->Int32Value()),
    .type = type.value(),
  };
  artik_error err = bt->set_scan_filter(&filter);

  for (auto & uuid : uuids_vector) {
    free(uuid.uuid);
  }

  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void BluetoothWrapper::set_alias(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  if (!args[0]->IsString()) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
      return;
  }

  String::Utf8Value param0(args[0]->ToString());
  char* alias = *param0;
  artik_error err = bt->set_alias(alias);

  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void BluetoothWrapper::set_discoverable(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  if (!args[0]->IsBoolean()) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
      return;
  }

  artik_error err = bt->set_discoverable(Boolean::Cast(*args[0])->Value());

  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void BluetoothWrapper::set_pairable(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  if (!args[0]->IsBoolean()) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
      return;
  }

  artik_error err = bt->set_pairable(Boolean::Cast(*args[0])->Value());

  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void BluetoothWrapper::set_pairableTimeout(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  if (!args[0]->IsUint32()) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
      return;
  }

  artik_error err = bt->set_pairableTimeout(Uint32::Cast(*args[0])->Value());

  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void BluetoothWrapper::set_discoverableTimeout(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  if (!args[0]->IsUint32()) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  artik_error err = bt->set_discoverableTimeout(
      Uint32::Cast(*args[0])->Value());

  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void BluetoothWrapper::is_scanning(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  bool is_scanning = bt->is_scanning();

  args.GetReturnValue().Set(Boolean::New(isolate, is_scanning));
}

void BluetoothWrapper::get_adapter_info(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder())->getObj();

  log_dbg("");
  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  artik_bt_adapter adapter;
  artik_error err = bt->get_adapter_info(&adapter);

  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }

  Local<Object> js_adapter = Object::New(isolate);
  js_adapter->Set(String::NewFromUtf8(isolate, "address"),
      String::NewFromUtf8(isolate, adapter.address));
  js_adapter->Set(String::NewFromUtf8(isolate, "name"),
      String::NewFromUtf8(isolate, adapter.name));
  js_adapter->Set(String::NewFromUtf8(isolate, "alias"),
      String::NewFromUtf8(isolate, adapter.alias));

  Local<Object> js_class = Object::New(isolate);

  std::string major_device_class = "unknow";
  std::string minor_device_class = "unknow";
  std::string service_class = "unknow";

  if (bt_major_device_class.find(adapter.cod.major) !=
      bt_major_device_class.end()) {
    major_device_class = bt_major_device_class.at(adapter.cod.major);
  }

  if (bt_minor_device_class.find(adapter.cod.major) !=
      bt_minor_device_class.end()) {
    minor_device_class = bt_minor_device_class.at(adapter.cod.minor);
  }

  if (bt_device_service_class.find(adapter.cod.major) !=
      bt_device_service_class.end()) {
    service_class = bt_device_service_class.at(adapter.cod.service_class);
  }

  js_class->Set(String::NewFromUtf8(isolate, "major"),
          String::NewFromUtf8(isolate, major_device_class.c_str()));
  js_class->Set(String::NewFromUtf8(isolate, "minor"),
          String::NewFromUtf8(isolate, minor_device_class.c_str()));
  js_class->Set(String::NewFromUtf8(isolate, "service_class"),
          String::NewFromUtf8(isolate, service_class.c_str()));
  js_adapter->Set(String::NewFromUtf8(isolate, "cod"),
      js_class);
  js_adapter->Set(String::NewFromUtf8(isolate, "discoverable"),
      Boolean::New(isolate, adapter.discoverable));
  js_adapter->Set(String::NewFromUtf8(isolate, "pairable"),
      Boolean::New(isolate, adapter.pairable));
  js_adapter->Set(String::NewFromUtf8(isolate, "pair_timeout"),
      Uint32::New(isolate, adapter.pair_timeout));
  js_adapter->Set(String::NewFromUtf8(isolate, "discover_timeout"),
      Uint32::New(isolate, adapter.discover_timeout));
  js_adapter->Set(String::NewFromUtf8(isolate, "discovering"),
      Boolean::New(isolate, adapter.discovering));

  Local<Array> js_uuids = Array::New(isolate);
  for (int i = 0; i < adapter.uuid_length; i++) {
    Local<Object> js_uuid = Object::New(isolate);
    js_uuid->Set(String::NewFromUtf8(isolate, "uuid"),
          String::NewFromUtf8(isolate, adapter.uuid_list[i].uuid));
    js_uuid->Set(String::NewFromUtf8(isolate, "uuid_name"),
          String::NewFromUtf8(isolate, adapter.uuid_list[i].uuid_name));
    js_uuids->Set(i, js_uuid);
  }

  js_adapter->Set(String::NewFromUtf8(isolate, "uuids"), js_uuids);

  args.GetReturnValue().Set(js_adapter);
}

void BluetoothWrapper::remove_devices(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  artik_error err = bt->remove_devices();

  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(String::NewFromUtf8(
        isolate, msg.c_str())));
    return;
  }
}

void BluetoothWrapper::connect_profile(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder())->getObj();

  log_dbg("");
  if (args.Length() != 2) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  if (!args[0]->IsString() || !args[1]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  String::Utf8Value param0(args[0]->ToString());
  char* addr = *param0;
  String::Utf8Value param1(args[1]->ToString());
  char* uuid = *param1;

  artik_error err = bt->connect_profile(addr, uuid);

  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void BluetoothWrapper::set_trust(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder())->getObj();

  log_dbg("%s", __func__);
  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
  }

  String::Utf8Value param0(args[0]->ToString());
  char* addr = *param0;

  artik_error err = bt->set_trust(addr);

  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void BluetoothWrapper::unset_trust(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder())->getObj();

  log_dbg("%s", __func__);

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  String::Utf8Value param0(args[0]->ToString());
  char* addr = *param0;

  artik_error err = bt->unset_trust(addr);

  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void BluetoothWrapper::set_block(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  String::Utf8Value param0(args[0]->ToString());
  char* addr = *param0;

  artik_error err = bt->set_block(addr);

  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void BluetoothWrapper::unset_block(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  String::Utf8Value param0(args[0]->ToString());
  char* addr = *param0;

  artik_error err = bt->unset_block(addr);

  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
          String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void BluetoothWrapper::is_paired(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  String::Utf8Value param0(args[0]->ToString());
  char* addr = *param0;

  bool is_paired = bt->is_paired(addr);
  args.GetReturnValue().Set(Boolean::New(isolate, is_paired));
}

void BluetoothWrapper::is_connected(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  String::Utf8Value param0(args[0]->ToString());
  char* addr = *param0;

  bool is_connected = bt->is_connected(addr);
  args.GetReturnValue().Set(Boolean::New(isolate, is_connected));
}

void BluetoothWrapper::is_trusted(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  String::Utf8Value param0(args[0]->ToString());
  char* addr = *param0;

  bool trusted = bt->is_trusted(addr);
  args.GetReturnValue().Set(Boolean::New(isolate, trusted));
}

void BluetoothWrapper::is_blocked(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<BluetoothWrapper>(args.Holder())->getObj();

  log_dbg("");
  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  String::Utf8Value param0(args[0]->ToString());
  char* addr = *param0;

  bool blocked = bt->is_blocked(addr);
  args.GetReturnValue().Set(Boolean::New(isolate, blocked));
}

}  // namespace artik

