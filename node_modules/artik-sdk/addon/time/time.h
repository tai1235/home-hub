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

#ifndef ADDON_TIME_TIME_H_
#define ADDON_TIME_TIME_H_

#include <node.h>
#include <nan.h>
#include <node_object_wrap.h>

#include <artik_time.hh>

#include <loop.h>

namespace artik {

class AlarmWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object>);

  Alarm* getObj() { return m_alarm; }
  v8::Persistent<v8::Function>* getAlarmCb() { return m_alarm_cb; }

  static v8::Local<v8::Object> NewInstance();

  void set_alarm(Alarm*);
  void set_alarm_cb(v8::Local<v8::Function>);

 private:
  explicit AlarmWrapper(Alarm*);
  AlarmWrapper();
  ~AlarmWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void get_delay(const v8::FunctionCallbackInfo<v8::Value>& args);

  Alarm *m_alarm;
  v8::Persistent<v8::Function>* m_alarm_cb;
};

class TimeWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object>);

  Time* getObj() { return m_time; }

 private:
  TimeWrapper();
  ~TimeWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void set_time(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_time(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_time_str(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_tick(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void sync_ntp(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void convert_timestamp_to_time(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void convert_time_to_timestamp(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void create_alarm_second(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void create_alarm_date(
      const v8::FunctionCallbackInfo<v8::Value>& args);

  Time  *m_time;
  GlibLoop* m_loop;
};

}  // namespace artik

#endif  // ADDON_TIME_TIME_H_

