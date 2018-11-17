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

#ifndef ADDON_BLUETOOTH_AVRCP_H_
#define ADDON_BLUETOOTH_AVRCP_H_

#include <node.h>
#include <node_object_wrap.h>

#include <artik_bluetooth.hh>
#include <loop.h>

#include <array>

namespace artik {

class AvrcpWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  Bluetooth* getObj() { return m_bt; }

 private:
  AvrcpWrapper();
  ~AvrcpWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);

  static v8::Persistent<v8::Function> constructor;
  static std::array<const char*, 4> bt_avrcp_repeat_mode;

  static void avrcp_controller_change_folder(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_list_item(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_get_repeat_mode(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_set_repeat_mode(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_is_connected(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_resume_play(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_pause(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_stop(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_next(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_previous(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_fast_forward(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_rewind(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_get_property(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_play_item(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_add_to_playing(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_get_name(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_get_status(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_get_subtype(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_get_type(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_is_browsable(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_get_position(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void avrcp_controller_get_metadata(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  Bluetooth *m_bt;
  GlibLoop *m_loop;
};

}  // namespace artik

#endif  // ADDON_BLUETOOTH_AVRCP_H_

