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

#include "bluetooth/avrcp.h"

#include <artik_log.h>
#include <glib.h>
#include <utils.h>

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
using v8::Boolean;
using v8::Null;
using v8::Context;

Persistent<Function> AvrcpWrapper::constructor;

std::array<const char*, 4> AvrcpWrapper::bt_avrcp_repeat_mode = {
  "singletrack",
  "alltracks",
  "group",
  "off"
};

static Local<Value> string_to_js_val(Isolate *isolate, char *str) {
  Local<Value> val;
  if (str != NULL) {
    val = String::NewFromUtf8(isolate, str);
  } else {
    val = Null(isolate);
  }

  return val;
}

static Local<Object> convert_item_property_to_json_object(Isolate *isolate,
    artik_bt_avrcp_item_property *property) {
  bool is_audio_or_video = (strcmp(property->type, "audio") == 0)
    || (strcmp(property->type, "video") == 0);

  Local<Object> js_property = Object::New(isolate);
  js_property->Set(String::NewFromUtf8(isolate, "player"),
       String::NewFromUtf8(isolate, property->player));
  js_property->Set(String::NewFromUtf8(isolate, "name"),
       String::NewFromUtf8(isolate, property->name));
  js_property->Set(String::NewFromUtf8(isolate, "type"),
       String::NewFromUtf8(isolate, property->type));
  js_property->Set(String::NewFromUtf8(isolate, "playable"),
       Boolean::New(isolate, property->playable));

  if (!is_audio_or_video) {
    js_property->Set(String::NewFromUtf8(isolate, "folder"),
         String::NewFromUtf8(isolate, property->folder));
  }

  if (is_audio_or_video) {
    js_property->Set(String::NewFromUtf8(isolate, "title"),
        string_to_js_val(isolate, property->title));
    js_property->Set(String::NewFromUtf8(isolate, "artist"),
        string_to_js_val(isolate, property->artist));
    js_property->Set(String::NewFromUtf8(isolate, "album"),
        string_to_js_val(isolate, property->album));
    js_property->Set(String::NewFromUtf8(isolate, "genre"),
        string_to_js_val(isolate, property->genre));
    js_property->Set(String::NewFromUtf8(isolate, "number_of_tracks"),
         Int32::New(isolate, property->number_of_tracks));
    js_property->Set(String::NewFromUtf8(isolate, "number"),
         Int32::New(isolate, property->number));
    js_property->Set(String::NewFromUtf8(isolate, "duration"),
         Int32::New(isolate, property->duration));
  }

  return js_property;
}

static Local<Object> convert_item_metadata_to_json_object(Isolate *isolate,
    artik_bt_avrcp_track_metadata *metadata) {
  Local<Object> js_property = Object::New(isolate);
  js_property->Set(String::NewFromUtf8(isolate, "title"),
      string_to_js_val(isolate, metadata->title));
  js_property->Set(String::NewFromUtf8(isolate, "artist"),
      string_to_js_val(isolate, metadata->artist));
  js_property->Set(String::NewFromUtf8(isolate, "album"),
      string_to_js_val(isolate, metadata->album));
  js_property->Set(String::NewFromUtf8(isolate, "genre"),
      string_to_js_val(isolate, metadata->genre));
  js_property->Set(String::NewFromUtf8(isolate, "number_of_tracks"),
      Int32::New(isolate, metadata->number_of_tracks));
  js_property->Set(String::NewFromUtf8(isolate, "number"),
      Int32::New(isolate, metadata->number));
  js_property->Set(String::NewFromUtf8(isolate, "duration"),
      Int32::New(isolate, metadata->duration));

  return js_property;
}

AvrcpWrapper::AvrcpWrapper()
  : m_bt(new Bluetooth()) {
  m_loop = GlibLoop::Instance();
  m_loop->attach();
}

AvrcpWrapper::~AvrcpWrapper() {
  delete m_bt;
  m_loop->detach();
}

void AvrcpWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "Avrcp"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_change_folder",
      avrcp_controller_change_folder);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_list_item",
      avrcp_controller_list_item);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_get_repeat_mode",
      avrcp_controller_get_repeat_mode);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_set_repeat_mode",
      avrcp_controller_set_repeat_mode);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_is_connected",
      avrcp_controller_is_connected);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_resume_play",
      avrcp_controller_resume_play);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_pause",
      avrcp_controller_pause);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_stop",
      avrcp_controller_stop);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_next",
      avrcp_controller_next);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_previous",
      avrcp_controller_previous);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_fast_forward",
      avrcp_controller_fast_forward);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_rewind",
      avrcp_controller_rewind);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_get_property",
      avrcp_controller_get_property);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_play_item",
      avrcp_controller_play_item);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_add_to_playing",
      avrcp_controller_add_to_playing);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_get_name",
      avrcp_controller_get_name);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_get_status",
      avrcp_controller_get_status);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_get_subtype",
      avrcp_controller_get_subtype);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_get_type",
      avrcp_controller_get_type);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_is_browsable",
      avrcp_controller_is_browsable);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_get_position",
      avrcp_controller_get_position);
  NODE_SET_PROTOTYPE_METHOD(tpl, "controller_get_metadata",
      avrcp_controller_get_metadata);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "Avrcp"),
         tpl->GetFunction());
}

void AvrcpWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  log_dbg("");

  if (args.IsConstructCall()) {
    AvrcpWrapper* obj = new AvrcpWrapper();
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

void AvrcpWrapper::avrcp_controller_change_folder(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments type")));
    return;
  }

  int index = args[0]->Int32Value();
  artik_error err = obj->avrcp_controller_change_folder(index);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void AvrcpWrapper::avrcp_controller_list_item(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 2) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsInt32() || !args[1]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments type")));
    return;
  }

  artik_bt_avrcp_item *items, *item;
  artik_error err = obj->avrcp_controller_list_item(args[0]->Int32Value(),
      args[1]->Int32Value(), &items);

  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
  item = items;

  int i = 0;
  Local<Array> js_items = Array::New(isolate);
  while (item != NULL) {
    artik_bt_avrcp_item_property *property = item->property;

    Local<Object> js_item = Object::New(isolate);

    js_item->Set(String::NewFromUtf8(isolate, "index"),
        Int32::New(isolate, item->index));
    js_item->Set(String::NewFromUtf8(isolate, "path"),
        String::NewFromUtf8(isolate, item->item_obj_path));
    Local<Object> js_property = convert_item_property_to_json_object(isolate,
        property);
    js_item->Set(String::NewFromUtf8(isolate, "property"), js_property);

    js_items->Set(i, js_item);
    item = item->next_item;
    i++;
  }
  obj->avrcp_controller_free_items(&items);
  args.GetReturnValue().Set(js_items);
}

void AvrcpWrapper::avrcp_controller_get_repeat_mode(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  }

  artik_bt_avrcp_repeat_mode mode;
  artik_error err = obj->avrcp_controller_get_repeat_mode(&mode);

  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }

  args.GetReturnValue().Set(
      String::NewFromUtf8(isolate, bt_avrcp_repeat_mode[mode]));
}

void AvrcpWrapper::avrcp_controller_set_repeat_mode(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments type")));
    return;
  }

  String::Utf8Value val(args[0]->ToString());
  auto mode =
    to_artik_parameter<artik_bt_avrcp_repeat_mode>(bt_avrcp_repeat_mode, *val);

  if (!mode) {
    std::string err = "Repeat mode " + std::string(*val) + " is not supported";
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, err.c_str())));
    return;
  }

  artik_error err = obj->avrcp_controller_set_repeat_mode(mode.value());
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void AvrcpWrapper::avrcp_controller_is_connected(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  }

  bool is_connected = obj->avrcp_controller_is_connected();
  args.GetReturnValue().Set(Boolean::New(isolate, is_connected));
}

void AvrcpWrapper::avrcp_controller_resume_play(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  }

  artik_error err = obj->avrcp_controller_resume_play();
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void AvrcpWrapper::avrcp_controller_pause(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  }

  artik_error err = obj->avrcp_controller_pause();
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void AvrcpWrapper::avrcp_controller_stop(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  }

  artik_error err = obj->avrcp_controller_stop();
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void AvrcpWrapper::avrcp_controller_next(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  }

  artik_error err = obj->avrcp_controller_next();
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void AvrcpWrapper::avrcp_controller_previous(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  }

  artik_error err = obj->avrcp_controller_previous();
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void AvrcpWrapper::avrcp_controller_fast_forward(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  }

  artik_error err = obj->avrcp_controller_fast_forward();
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void AvrcpWrapper::avrcp_controller_rewind(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  }

  artik_error err = obj->avrcp_controller_rewind();
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void AvrcpWrapper::avrcp_controller_get_property(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments type")));
    return;
  }

  artik_bt_avrcp_item_property *property = NULL;
  int index = args[0]->Int32Value();
  artik_error err = obj->avrcp_controller_get_property(index, &property);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }

  Local<Object> js_property = convert_item_property_to_json_object(
      isolate, property);
  args.GetReturnValue().Set(js_property);
  obj->avrcp_controller_free_property(&property);
}

void AvrcpWrapper::avrcp_controller_play_item(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");
  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments type")));
    return;
  }

  int index = args[0]->Int32Value();
  artik_error err = obj->avrcp_controller_play_item(index);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void AvrcpWrapper::avrcp_controller_add_to_playing(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsInt32()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments type")));
    return;
  }

  int index = args[0]->Int32Value();
  artik_error err = obj->avrcp_controller_add_to_playing(index);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void AvrcpWrapper::avrcp_controller_get_name(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");
  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  char *name;
  artik_error err = obj->avrcp_controller_get_name(&name);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }

  Local<String> js_name = String::NewFromUtf8(isolate, name);
  args.GetReturnValue().Set(js_name);
  g_free(name);
}

void AvrcpWrapper::avrcp_controller_get_status(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  char *status;
  artik_error err = obj->avrcp_controller_get_status(&status);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }

  Local<String> js_status = String::NewFromUtf8(isolate, status);
  args.GetReturnValue().Set(js_status);
  g_free(status);
}

void AvrcpWrapper::avrcp_controller_get_subtype(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  char *subtype;
  artik_error err = obj->avrcp_controller_get_subtype(&subtype);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }

  Local<String> js_subtype = String::NewFromUtf8(isolate, subtype);
  args.GetReturnValue().Set(js_subtype);
  g_free(subtype);
}

void AvrcpWrapper::avrcp_controller_get_type(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  char *type;
  artik_error err = obj->avrcp_controller_get_type(&type);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }

  Local<String> js_type = String::NewFromUtf8(isolate, type);
  args.GetReturnValue().Set(js_type);
  g_free(type);
}

void AvrcpWrapper::avrcp_controller_is_browsable(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");
  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  bool browsable = obj->avrcp_controller_is_browsable();

  Local<Boolean> js_browsable = Boolean::New(isolate, browsable);
  args.GetReturnValue().Set(js_browsable);
}

void AvrcpWrapper::avrcp_controller_get_position(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  unsigned int position;
  artik_error err = obj->avrcp_controller_get_position(&position);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }

  args.GetReturnValue().Set(Int32::New(isolate, position));
}

void AvrcpWrapper::avrcp_controller_get_metadata(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<AvrcpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  artik_bt_avrcp_track_metadata *meta = NULL;
  artik_error err = obj->avrcp_controller_get_metadata(&meta);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }

  Local<Object> js_metadata = convert_item_metadata_to_json_object(
      isolate, meta);
  args.GetReturnValue().Set(js_metadata);
  obj->avrcp_controller_free_metadata(&meta);
}
}  // namespace artik

