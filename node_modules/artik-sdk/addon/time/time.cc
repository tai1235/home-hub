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

#include "time/time.h"

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
using v8::Integer;
using v8::Date;
using v8::External;
using v8::Handle;
using v8::HandleScope;
using v8::Script;
using v8::Context;

Persistent<Function> TimeWrapper::constructor;
Persistent<Function> AlarmWrapper::constructor;

static void _alarm_callback(void *user_data) {
  Isolate* isolate = Isolate::GetCurrent();
  HandleScope handleScope(isolate);
  AlarmWrapper* wrap = reinterpret_cast<AlarmWrapper*>(user_data);

  if (!wrap->getAlarmCb())
    return;

  Local<Function>::New(isolate, *wrap->getAlarmCb())->Call(
      isolate->GetCurrentContext()->Global(), 0, NULL);
}

AlarmWrapper::AlarmWrapper(Alarm *value) {
  this->m_alarm = value;
}

AlarmWrapper::AlarmWrapper() {
  this->m_alarm = new Alarm();
}

AlarmWrapper::~AlarmWrapper() {
  if (m_alarm)
    delete m_alarm;
}

void AlarmWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();
  Local<FunctionTemplate> modal = FunctionTemplate::New(isolate, New);
  modal->SetClassName(String::NewFromUtf8(isolate, "alarm"));
  modal->InstanceTemplate()->SetInternalFieldCount(1);

  NODE_SET_PROTOTYPE_METHOD(modal, "get_delay", get_delay);

  constructor.Reset(isolate, modal->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "alarm"), modal->GetFunction());
}

void AlarmWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  int lenArg = 0;

  if (args.Length() > lenArg) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (args.IsConstructCall()) {
    AlarmWrapper* obj = new AlarmWrapper();
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

Local<Object> AlarmWrapper::NewInstance() {
  Isolate * isolate = Isolate::GetCurrent();
  Local<Context> context = isolate->GetCurrentContext();
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  Local<Object> obj = cons->NewInstance(context, 0, NULL).ToLocalChecked();
  return obj;
}

void AlarmWrapper::get_delay(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }
  Alarm* obj = ObjectWrap::Unwrap<AlarmWrapper>(args.Holder())->getObj();
  artik_msecond msecond;
  artik_error ret = obj->get_delay(&msecond);
  if (ret != S_OK) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, error_msg(ret))));
    return;
  }
  args.GetReturnValue().Set(Number::New(isolate, msecond));
}

void AlarmWrapper::set_alarm(Alarm *alarm) {
  m_alarm = new Alarm(*alarm);
}

void AlarmWrapper::set_alarm_cb(Local<Function> callback) {
  Isolate * isolate = Isolate::GetCurrent();
  m_alarm_cb = new Persistent<Function>();
  m_alarm_cb->Reset(isolate, callback);
}

TimeWrapper::TimeWrapper() {
  m_time = new Time();
  this->m_loop = GlibLoop::Instance();
  this->m_loop->attach();
}

TimeWrapper::~TimeWrapper() {
  delete m_time;
  this->m_loop->detach();
}

void TimeWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();
  Local<FunctionTemplate> modal = FunctionTemplate::New(isolate, New);
  modal->SetClassName(String::NewFromUtf8(isolate, "time"));
  modal->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(modal, "set_time", set_time);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_time", get_time);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_time_str", get_time_str);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_tick", get_tick);
  NODE_SET_PROTOTYPE_METHOD(modal, "create_alarm_second", create_alarm_second);
  NODE_SET_PROTOTYPE_METHOD(modal, "create_alarm_date", create_alarm_date);
  NODE_SET_PROTOTYPE_METHOD(modal, "sync_ntp", sync_ntp);
  NODE_SET_PROTOTYPE_METHOD(modal, "convert_timestamp_to_time",
                                    convert_timestamp_to_time);
  NODE_SET_PROTOTYPE_METHOD(modal, "convert_time_to_timestamp",
                                    convert_time_to_timestamp);

  constructor.Reset(isolate, modal->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "time"), modal->GetFunction());
  AlarmWrapper::Init(exports);
}

void TimeWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  int lenArg = 0;

  if (args.Length() != lenArg) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }
  if (args.IsConstructCall()) {
    TimeWrapper* obj = NULL;
    obj = new TimeWrapper();
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Value> argv[lenArg];

    if (args.Length() == lenArg) {
      for (int i = 0; i < lenArg; ++i)
        argv[i] = args[i];
    }
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, args.Length(), argv).ToLocalChecked());
  }
}

void TimeWrapper::set_time(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsDate()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Invalid argument")));
    return;
  }

  Local<Value> tab_args[0] = NULL;
  Time* obj = ObjectWrap::Unwrap<TimeWrapper>(args.Holder())->getObj();
  Local<Object> object = Local<Object>::Cast(args[0]->ToObject());
  artik_time date;

  memset(&date, 0, sizeof(date));
  date.second = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCSeconds")))->Call(object, 0, tab_args)->Int32Value();
  date.minute = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCMinutes")))->Call(object, 0, tab_args)->Int32Value();
  date.hour = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCHours")))->Call(object, 0, tab_args)->Int32Value();
  date.day = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCDate")))->Call(object, 0, tab_args)->Int32Value();
  date.month = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCMonth")))->Call(object, 0, tab_args)->Int32Value() + 1;
  date.year = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCFullYear")))->Call(object, 0, tab_args)->Int32Value();
  date.day_of_week = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCDay")))->Call(object, 0, tab_args)->Int32Value();
  date.msecond = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCMilliseconds")))->Call(object, 0, tab_args)->Int32Value();

  args.GetReturnValue().Set(
      Number::New(isolate, obj->set_time(date, ARTIK_TIME_UTC)));
}

void TimeWrapper::get_time(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  Time* obj = ObjectWrap::Unwrap<TimeWrapper>(args.Holder())->getObj();
  artik_time date;
  artik_error ret = obj->get_time(ARTIK_TIME_UTC, &date);
  if (ret != S_OK) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, error_msg(ret))));
    return;
  }

  char str[128];
  snprintf(str, sizeof(str),
      "new Date(Date.UTC(%d, %.2d, %.2d, %.2d, %.2d, %.2d))",
      date.year, date.month-1, date.day, date.hour, date.minute, date.second);

  Handle<String> src = String::NewFromUtf8(isolate,
      reinterpret_cast<char*>(str));
  Handle<Script> script = Script::Compile(src);
  Handle<Value> res = script->Run();
  args.GetReturnValue().Set(res);
}

void TimeWrapper::get_time_str(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 2) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }
  Time* obj = ObjectWrap::Unwrap<TimeWrapper>(args.Holder())->getObj();
  char date[128] = "";
  obj->get_time_str(date, 128, *String::Utf8Value(args[0]->ToString()),
      (artik_time_zone)args[1]->Int32Value());
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, date));
}

void TimeWrapper::get_tick(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }
  Time* obj = ObjectWrap::Unwrap<TimeWrapper>(args.Holder())->getObj();

  args.GetReturnValue().Set(Number::New(isolate, obj->get_tick()));
}

void TimeWrapper::sync_ntp(const FunctionCallbackInfo<v8::Value>& args) {
    Isolate* isolate = args.GetIsolate();
    if (args.Length() != 1) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Wrong number of arguments")));
      return;
    }

    String::Utf8Value param0(args[0]->ToString());
    char* hostname = *param0;
    Time* obj = ObjectWrap::Unwrap<TimeWrapper>(args.Holder())->getObj();
    args.GetReturnValue().Set(Number::New(isolate, obj->sync_ntp(hostname)));
}

void TimeWrapper::convert_timestamp_to_time(
                                  const FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsNumber()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Invalid argument")));
    return;
  }

  int64_t timestamp = args[0]->IntegerValue();
  artik_time date;
  Time* obj = ObjectWrap::Unwrap<TimeWrapper>(args.Holder())->getObj();

  artik_error ret = obj->convert_timestamp_to_time(timestamp, &date);
  if (ret != S_OK) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, error_msg(ret))));
    return;
  }

  char str[128];
  snprintf(str, sizeof(str),
      "new Date(Date.UTC(%d, %.2d, %.2d, %.2d, %.2d, %.2d))",
      date.year, date.month-1, date.day, date.hour, date.minute, date.second);

  Handle<String> src = String::NewFromUtf8(isolate,
      reinterpret_cast<char*>(str));
  Handle<Script> script = Script::Compile(src);
  Handle<Value> res = script->Run();
  args.GetReturnValue().Set(res);
}

void TimeWrapper::convert_time_to_timestamp(
                                  const FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsDate()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Invalid argument")));
    return;
  }

  Local<Value> tab_args[0] = NULL;
  Time* obj = ObjectWrap::Unwrap<TimeWrapper>(args.Holder())->getObj();
  Local<Object> object = Local<Object>::Cast(args[0]->ToObject());
  artik_time date;
  int64_t timestamp;

  memset(&date, 0, sizeof(date));
  date.second = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCSeconds")))->Call(object, 0, tab_args)->Int32Value();
  date.minute = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCMinutes")))->Call(object, 0, tab_args)->Int32Value();
  date.hour = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCHours")))->Call(object, 0, tab_args)->Int32Value();
  date.day = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCDate")))->Call(object, 0, tab_args)->Int32Value();
  date.month = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCMonth")))->Call(object, 0, tab_args)->Int32Value() + 1;
  date.year = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCFullYear")))->Call(object, 0, tab_args)->Int32Value();
  date.day_of_week = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCDay")))->Call(object, 0, tab_args)->Int32Value();
  date.msecond = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCMilliseconds")))->Call(object, 0, tab_args)->Int32Value();

  artik_error ret = obj->convert_time_to_timestamp(&date, &timestamp);
  if (ret != S_OK) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, error_msg(ret))));
    return;
  }

  args.GetReturnValue().Set(Number::New(isolate, timestamp));
}

void TimeWrapper::create_alarm_second(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  TimeWrapper *wrap = ObjectWrap::Unwrap<TimeWrapper>(args.Holder());

  if (args.Length() != 3) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  Time* obj = wrap->getObj();
  Local<Object> js_alarm = AlarmWrapper::NewInstance();
  AlarmWrapper* alarmWrap = ObjectWrap::Unwrap<AlarmWrapper>(js_alarm);

  alarmWrap->set_alarm_cb(Local<Function>::Cast(args[2]));
  Alarm *alarm = obj->create_alarm_second(
      (artik_time_zone)args[0]->Int32Value(),
      (artik_msecond)args[1]->NumberValue(),
      _alarm_callback,
      reinterpret_cast<void*>(alarmWrap));
  alarmWrap->set_alarm(alarm);
  args.GetReturnValue().Set(js_alarm);
}

void TimeWrapper::create_alarm_date(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  TimeWrapper *wrap = ObjectWrap::Unwrap<TimeWrapper>(args.Holder());

  if (args.Length() != 3) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  Time* obj = wrap->getObj();
  Local<Object> js_alarm = AlarmWrapper::NewInstance();
  AlarmWrapper* alarmWrap = ObjectWrap::Unwrap<AlarmWrapper>(js_alarm);

  Local<Value> tab_args[0] = NULL;
  Local<Object> object = Local<Object>::Cast(args[1]->ToObject());
  artik_time date;

  memset(&date, 0, sizeof(date));
  date.second = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCSeconds")))->Call(object, 0, tab_args)->Int32Value();
  date.minute = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCMinutes")))->Call(object, 0, tab_args)->Int32Value();
  date.hour = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCHours")))->Call(object, 0, tab_args)->Int32Value() +
      args[0]->Int32Value();
  date.day = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCDate")))->Call(object, 0, tab_args)->Int32Value();
  date.month = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCMonth")))->Call(object, 0, tab_args)->Int32Value() + 1;
  date.year = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCFullYear")))->Call(object, 0, tab_args)->Int32Value();
  date.day_of_week = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCDay")))->Call(object, 0, tab_args)->Int32Value();
  date.msecond = Local<Function>::Cast(object->Get(String::NewFromUtf8(
      isolate, "getUTCMilliseconds")))->Call(object, 0, tab_args)->Int32Value();

  alarmWrap->set_alarm_cb(Local<Function>::Cast(args[2]));
  Alarm *alarm = obj->create_alarm_date(
      (artik_time_zone)args[0]->Int32Value(), date,
      _alarm_callback, reinterpret_cast<void*>(alarmWrap));
  alarmWrap->set_alarm(alarm);
  args.GetReturnValue().Set(js_alarm);
}

}  // namespace artik

