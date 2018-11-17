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

#ifndef ADDON_ZIGBEE_ZIGBEE_DEVICE_H_
#define ADDON_ZIGBEE_ZIGBEE_DEVICE_H_

#include <node.h>
#include <node_object_wrap.h>
#include <uv.h>
#include <artik_zigbee.hh>

using v8::Function;
using v8::Local;
using v8::Isolate;

namespace artik {

class OnOffLightWrapper: public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  OnOffLightDevice* getObj() { return m_zbd; }

 private:
  OnOffLightWrapper();
  ~OnOffLightWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void groups_get_local_name_support(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void groups_set_local_name_support(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void onoff_get_value(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ezmode_commissioning_target_start(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ezmode_commissioning_target_stop(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  OnOffLightDevice* m_zbd;
};

class OnOffSwitchWrapper: public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  OnOffSwitchDevice* getObj() { return m_zbd; }

 private:
  OnOffSwitchWrapper();
  ~OnOffSwitchWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void identify_request(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void identify_get_remaining_time(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void onoff_command(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ezmode_commissioning_initiator_start(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ezmode_commissioning_initiator_stop(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  OnOffSwitchDevice* m_zbd;
};

class LevelControlSwitchWrapper: public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  LevelControlSwitchDevice* getObj() { return m_zbd; }

 private:
  LevelControlSwitchWrapper();
  ~LevelControlSwitchWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void identify_request(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void identify_get_remaining_time(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void onoff_command(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void level_control_request(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ezmode_commissioning_initiator_start(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ezmode_commissioning_initiator_stop(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  LevelControlSwitchDevice* m_zbd;
};

class DimmableLightWrapper: public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  DimmableLightDevice* getObj() { return m_zbd; }

 private:
  DimmableLightWrapper();
  ~DimmableLightWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void groups_get_local_name_support(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void groups_set_local_name_support(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void onoff_get_value(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void level_control_get_value(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ezmode_commissioning_target_start(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ezmode_commissioning_target_stop(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  DimmableLightDevice* m_zbd;
};

class LightSensorWrapper: public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  LightSensorDevice* getObj() { return m_zbd; }

 private:
  LightSensorWrapper();
  ~LightSensorWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void identify_request(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void identify_get_remaining_time(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void illum_set_measured_value_range(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void illum_set_measured_value(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void illum_get_measured_value(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ezmode_commissioning_initiator_start(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ezmode_commissioning_initiator_stop(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  LightSensorDevice* m_zbd;
};

class RemoteControlWrapper: public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  RemoteControlDevice* getObj() { return m_zbd; }

 private:
  RemoteControlWrapper();
  ~RemoteControlWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void reset_to_factory_default(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void identify_request(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void identify_get_remaining_time(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void onoff_command(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void level_control_request(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void request_reporting(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void stop_reporting(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ezmode_commissioning_target_start(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void ezmode_commissioning_target_stop(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  RemoteControlDevice* m_zbd;
};

}  // namespace artik

#endif  // ADDON_ZIGBEE_ZIGBEE_DEVICE_H_
