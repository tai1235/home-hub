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

#include "sensor/sensor.h"

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
using v8::External;
using v8::Context;

Persistent<Function> SensorWrapper::constructor;
Persistent<Function> SensorDeviceWrapper::constructor;
Persistent<Function> AccelerometerWrapper::constructor;
Persistent<Function> HumidityWrapper::constructor;
Persistent<Function> LightWrapper::constructor;
Persistent<Function> TemperatureWrapper::constructor;
Persistent<Function> ProximityWrapper::constructor;
Persistent<Function> FlameWrapper::constructor;
Persistent<Function> GyroWrapper::constructor;
Persistent<Function> PressureWrapper::constructor;
Persistent<Function> HallWrapper::constructor;

v8::Handle<FunctionTemplate> SensorDeviceWrapper::modal;
v8::Handle<FunctionTemplate> AccelerometerWrapper::modal;
v8::Handle<FunctionTemplate> HumidityWrapper::modal;
v8::Handle<FunctionTemplate> LightWrapper::modal;
v8::Handle<FunctionTemplate> TemperatureWrapper::modal;
v8::Handle<FunctionTemplate> ProximityWrapper::modal;
v8::Handle<FunctionTemplate> FlameWrapper::modal;
v8::Handle<FunctionTemplate> PressureWrapper::modal;
v8::Handle<FunctionTemplate> GyroWrapper::modal;
v8::Handle<FunctionTemplate> HallWrapper::modal;

// SENSORDEVICE BASE IMPLEMENTATION

SensorDeviceWrapper::SensorDeviceWrapper(SensorDevice *obj) {
  m_sensor = obj;
}

SensorDeviceWrapper::SensorDeviceWrapper() {
  m_sensor = NULL;
}

SensorDeviceWrapper::~SensorDeviceWrapper() {
  SensorDeviceWrapper::clean(m_sensor);
}

void SensorDeviceWrapper::clean(void *elem) {
  SensorDevice *sensor = reinterpret_cast<SensorDevice*>(elem);
  if (sensor)
    delete sensor;
}

void SensorDeviceWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();
  modal = FunctionTemplate::New(isolate, New);
  modal->SetClassName(String::NewFromUtf8(isolate, "SensorDevice"));
  modal->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(modal, "get_type", get_type);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_index", get_index);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_name", get_name);

  constructor.Reset(isolate, modal->GetFunction());
  exports->Set(v8::String::NewFromUtf8(isolate, "SensorDevice"),
      modal->GetFunction());
  AccelerometerWrapper::Init(exports);
  GyroWrapper::Init(exports);
  HumidityWrapper::Init(exports);
  LightWrapper::Init(exports);
  TemperatureWrapper::Init(exports);
  ProximityWrapper::Init(exports);
  FlameWrapper::Init(exports);
  PressureWrapper::Init(exports);
  HallWrapper::Init(exports);
}

void SensorDeviceWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  int lenArg = 1;
  if (args.Length() != lenArg && args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  if (args.IsConstructCall()) {
    SensorDeviceWrapper* obj = NULL;
    if (args.Length() == lenArg) {
      Local<External> ext = Local<External>::Cast(args[0]);
      obj = new SensorDeviceWrapper(
          reinterpret_cast<SensorDevice*>(ext->Value()));
    } else {
      obj = new SensorDeviceWrapper();
    }
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    int argc = args.Length();
    Local<Value> argv[argc];

    for (int i = 0; i < argc; ++i)
        argv[i] = args[i];

    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, argc, argv).ToLocalChecked());
  }
}

void SensorDeviceWrapper::NewArrayInstance(
    const v8::FunctionCallbackInfo<v8::Value>& args,
    std::vector<SensorDevice*> const &value) {
  Isolate * isolate = Isolate::GetCurrent();
  Local<Array> result_list = Array::New(isolate);

  for (unsigned int i = 0; i < value.size(); i++) {
    v8::HandleScope scope(isolate);
    Local<Value> argv[1] = {
      External::New(isolate, reinterpret_cast<void*>(value[i]))
    };
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    result_list->Set(i, cons->NewInstance(context, 1, argv).ToLocalChecked());
  }
  args.GetReturnValue().Set(result_list);
}

void SensorDeviceWrapper::NewInstance(
    const v8::FunctionCallbackInfo<v8::Value>& args, SensorDevice *value) {
  Isolate * isolate = Isolate::GetCurrent();
  Nan::EscapableHandleScope scope;
  Local<Value> argv[1] = {
    External::New(isolate, reinterpret_cast<void*>(value))
  };
  Local<Context> context = isolate->GetCurrentContext();
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  Local<Object> obj = cons->NewInstance(context, 1, argv).ToLocalChecked();
  args.GetReturnValue().Set(obj);
}

void SensorDeviceWrapper::get_type(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  SensorDevice* obj = ObjectWrap::Unwrap<SensorDeviceWrapper>(
      args.Holder())->getObj();

  args.GetReturnValue().Set(Number::New(isolate, obj->get_type()));
}

void SensorDeviceWrapper::get_index(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  SensorDevice* obj = ObjectWrap::Unwrap<SensorDeviceWrapper>(
      args.Holder())->getObj();
  args.GetReturnValue().Set(Number::New(isolate, obj->get_index()));
}

void SensorDeviceWrapper::get_name(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  SensorDevice* obj = ObjectWrap::Unwrap<SensorDeviceWrapper>(
      args.Holder())->getObj();

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, obj->get_name()));
}

// ACCELEROMETER DEVICE IMPLEMENTATION

AccelerometerWrapper::AccelerometerWrapper(AccelerometerSensor *obj) {
  m_sensor = obj;
}

AccelerometerWrapper::AccelerometerWrapper() {
  m_sensor = NULL;
}

AccelerometerWrapper::~AccelerometerWrapper() {
  AccelerometerWrapper::clean(m_sensor);
}

void AccelerometerWrapper::clean(void *elem) {
  AccelerometerSensor *sensor = reinterpret_cast<AccelerometerSensor*>(elem);
  if (sensor)
    delete sensor;
}

void AccelerometerWrapper::Init(Local<Object> exports) {
  Isolate *isolate = exports->GetIsolate();
  modal = FunctionTemplate::New(isolate, New);

  modal->Inherit(SensorDeviceWrapper::modal);
  modal->SetClassName(String::NewFromUtf8(isolate, "Accelerometer"));
  modal->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(modal, "get_speed_x", get_speed_x);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_speed_y", get_speed_y);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_speed_z", get_speed_z);

  constructor.Reset(isolate, modal->GetFunction());
  exports->Set(v8::String::NewFromUtf8(isolate, "Accelerometer"),
      modal->GetFunction());
}

void AccelerometerWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  int lenArg = 1;
  if (args.Length() != lenArg && args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  if (args.IsConstructCall()) {
    AccelerometerWrapper* obj = NULL;
    if (args.Length() == lenArg) {
      Local<External> ext = Local<External>::Cast(args[0]);
      obj = new AccelerometerWrapper(
          reinterpret_cast<AccelerometerSensor*>(ext->Value()));
    } else {
      obj = new AccelerometerWrapper();
    }
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

void AccelerometerWrapper::NewInstance(
    const v8::FunctionCallbackInfo<v8::Value>& args,
    AccelerometerSensor *value) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope scope(isolate);
  node::AtExit(clean, reinterpret_cast<void*>(value));
  Local<Value> argv[1] = {
    External::New(isolate, reinterpret_cast<void*>(value))
  };
  Local<Context> context = isolate->GetCurrentContext();
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  Local<Object> obj = cons->NewInstance(context, 1, argv).ToLocalChecked();
  args.GetReturnValue().Set(obj);
}

void AccelerometerWrapper::get_speed_x(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  AccelerometerSensor* obj = ObjectWrap::Unwrap<AccelerometerWrapper>(
      args.Holder())->getObj();

  args.GetReturnValue().Set(Number::New(isolate, obj->get_speed_x()));
}

void AccelerometerWrapper::get_speed_y(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  AccelerometerSensor* obj = ObjectWrap::Unwrap<AccelerometerWrapper>(
      args.Holder())->getObj();

  args.GetReturnValue().Set(Number::New(isolate, obj->get_speed_y()));
}

void AccelerometerWrapper::get_speed_z(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  AccelerometerSensor* obj = ObjectWrap::Unwrap<AccelerometerWrapper>(
      args.Holder())->getObj();

  args.GetReturnValue().Set(Number::New(isolate, obj->get_speed_z()));
}

// GYROMETER DEVICE IMPLEMENTATION

GyroWrapper::GyroWrapper(GyroSensor *obj) {
  m_sensor = obj;
}

GyroWrapper::GyroWrapper() {
  m_sensor = NULL;
}

GyroWrapper::~GyroWrapper() {
  GyroWrapper::clean(m_sensor);
}

void GyroWrapper::clean(void *elem) {
  GyroSensor *sensor = reinterpret_cast<GyroSensor*>(elem);
  if (sensor)
    delete sensor;
}

void GyroWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();
  modal = FunctionTemplate::New(isolate, New);

  modal->Inherit(SensorDeviceWrapper::modal);
  modal->SetClassName(String::NewFromUtf8(isolate, "Gyro"));
  modal->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(modal, "get_yaw", get_yaw);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_pitch", get_pitch);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_roll", get_roll);

  constructor.Reset(isolate, modal->GetFunction());
  exports->Set(v8::String::NewFromUtf8(isolate, "Gyro"),
      modal->GetFunction());
}


void GyroWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  int lenArg = 1;
  if (args.Length() != lenArg && args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  if (args.IsConstructCall()) {
    GyroWrapper* obj = NULL;
    if (args.Length() == lenArg) {
      Local<External> ext = Local<External>::Cast(args[0]);
      obj = new GyroWrapper(reinterpret_cast<GyroSensor*>(ext->Value()));
    } else {
      obj = new GyroWrapper();
    }
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

void GyroWrapper::NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args,
    GyroSensor *value) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope scope(isolate);
  node::AtExit(clean, reinterpret_cast<void*>(value));
  Local<Value> argv[1] = {
    External::New(isolate, reinterpret_cast<void*>(value))
  };
  Local<Context> context = isolate->GetCurrentContext();
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  Local<Object> obj = cons->NewInstance(context, 1, argv).ToLocalChecked();
  args.GetReturnValue().Set(obj);
}

void GyroWrapper::get_yaw(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  GyroSensor* obj = ObjectWrap::Unwrap<GyroWrapper>(args.Holder())->getObj();

  args.GetReturnValue().Set(Number::New(isolate, obj->get_yaw()));
}

void GyroWrapper::get_pitch(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  GyroSensor* obj = ObjectWrap::Unwrap<GyroWrapper>(args.Holder())->getObj();

  args.GetReturnValue().Set(Number::New(isolate, obj->get_pitch()));
}

void GyroWrapper::get_roll(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  GyroSensor* obj = ObjectWrap::Unwrap<GyroWrapper>(args.Holder())->getObj();

  args.GetReturnValue().Set(Number::New(isolate, obj->get_roll()));
}

// HUMIDITY DEVICE IMPLEMENTATION

HumidityWrapper::HumidityWrapper(HumiditySensor *obj) {
  m_sensor = obj;
}

HumidityWrapper::HumidityWrapper() {
  m_sensor = NULL;
}

HumidityWrapper::~HumidityWrapper() {
  HumidityWrapper::clean(m_sensor);
}

void HumidityWrapper::clean(void *elem) {
  HumiditySensor *sensor = reinterpret_cast<HumiditySensor*>(elem);
  if (sensor)
    delete sensor;
}

void HumidityWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  modal = FunctionTemplate::New(isolate, New);
  modal->Inherit(SensorDeviceWrapper::modal);
  modal->SetClassName(String::NewFromUtf8(isolate, "Humidity"));
  modal->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(modal, "get_humidity", get_humidity);

  constructor.Reset(isolate, modal->GetFunction());
  exports->Set(v8::String::NewFromUtf8(isolate, "Humidity"),
      modal->GetFunction());
}


void HumidityWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  int lenArg = 1;
  if (args.Length() != lenArg && args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  if (args.IsConstructCall()) {
    HumidityWrapper* obj = NULL;
    if (args.Length() == lenArg) {
      Local<External> ext = Local<External>::Cast(args[0]);
      obj = new HumidityWrapper(reinterpret_cast<HumiditySensor*>(
            ext->Value()));
    } else {
      obj = new HumidityWrapper();
    }
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

void HumidityWrapper::NewInstance(
    const v8::FunctionCallbackInfo<v8::Value>& args, HumiditySensor *value) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope scope(isolate);
  node::AtExit(clean, reinterpret_cast<void*>(value));
  Local<Value> argv[1] = {
    External::New(isolate, reinterpret_cast<void*>(value))
  };
  Local<Context> context = isolate->GetCurrentContext();
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  Local<Object> obj = cons->NewInstance(context, 1, argv).ToLocalChecked();
  args.GetReturnValue().Set(obj);
}

void HumidityWrapper::get_humidity(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  HumiditySensor* obj = ObjectWrap::Unwrap<HumidityWrapper>(
      args.Holder())->getObj();

  args.GetReturnValue().Set(Number::New(isolate, obj->get_humidity()));
}

// LIGHT DEVICE IMPLEMENTATION

LightWrapper::LightWrapper(LightSensor *obj) {
  m_sensor = obj;
}

LightWrapper::LightWrapper() {
  m_sensor = NULL;
}

LightWrapper::~LightWrapper() {
  LightWrapper::clean(m_sensor);
}

void LightWrapper::clean(void*elem) {
  LightSensor *sensor = reinterpret_cast<LightSensor*>(elem);
  if (sensor) {
    delete sensor;
  }
}

void LightWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  modal = FunctionTemplate::New(isolate, New);
  modal->Inherit(SensorDeviceWrapper::modal);
  modal->SetClassName(String::NewFromUtf8(isolate, "Light"));
  modal->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(modal, "get_intensity", get_intensity);
  constructor.Reset(isolate, modal->GetFunction());
  exports->Set(v8::String::NewFromUtf8(isolate, "Light"),
      modal->GetFunction());
}

void LightWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  int lenArg = 1;
  if (args.Length() != lenArg && args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  if (args.IsConstructCall()) {
    LightWrapper* obj = NULL;
    if (args.Length() == lenArg) {
      Local<External> ext = Local<External>::Cast(args[0]);
      obj = new LightWrapper(reinterpret_cast<LightSensor*>(ext->Value()));
    } else {
      obj = new LightWrapper();
    }
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

void LightWrapper::NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args,
    LightSensor *value) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope scope(isolate);
  node::AtExit(clean, reinterpret_cast<void*>(value));
  Local<Value> argv[1] = {
    External::New(isolate, reinterpret_cast<void*>(value))
  };
  Local<Context> context = isolate->GetCurrentContext();
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  args.GetReturnValue().Set(
      cons->NewInstance(context, 1, argv).ToLocalChecked());
}

void LightWrapper::get_intensity(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  LightSensor* obj = ObjectWrap::Unwrap<LightWrapper>(args.Holder())->getObj();

  args.GetReturnValue().Set(Number::New(isolate, obj->get_intensity()));
}

// TEMPERATURE DEVICE IMPLEMENTATION

TemperatureWrapper::TemperatureWrapper(TemperatureSensor *obj) {
  m_sensor = obj;
}

TemperatureWrapper::TemperatureWrapper() {
  m_sensor = NULL;
}

TemperatureWrapper::~TemperatureWrapper() {
  TemperatureWrapper::clean(m_sensor);
}

void TemperatureWrapper::clean(void *elem) {
  TemperatureSensor *sensor = reinterpret_cast<TemperatureSensor*>(elem);
  if (sensor)
    delete sensor;
}

void TemperatureWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  modal = FunctionTemplate::New(isolate, New);
  modal->Inherit(SensorDeviceWrapper::modal);
  modal->SetClassName(String::NewFromUtf8(isolate, "Temperature"));
  modal->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(modal, "get_celsius", get_celsius);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_fahrenheit", get_fahrenheit);

  constructor.Reset(isolate, modal->GetFunction());
  exports->Set(v8::String::NewFromUtf8(isolate, "Temperature"),
      modal->GetFunction());
}

void TemperatureWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  int lenArg = 1;
  if (args.Length() != lenArg && args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  if (args.IsConstructCall()) {
    TemperatureWrapper* obj = NULL;
    if (args.Length() == lenArg) {
      Local<External> ext = Local<External>::Cast(args[0]);
      obj = new TemperatureWrapper(
          reinterpret_cast<TemperatureSensor*>(ext->Value()));
    } else {
      obj = new TemperatureWrapper();
    }
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

void TemperatureWrapper::NewInstance(
    const v8::FunctionCallbackInfo<v8::Value>& args, TemperatureSensor *value) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope scope(isolate);
  node::AtExit(clean, reinterpret_cast<void*>(value));
  Local<Value> argv[1] = {
    External::New(isolate, reinterpret_cast<void*>(value))
  };
  Local<Context> context = isolate->GetCurrentContext();
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  args.GetReturnValue().Set(
      cons->NewInstance(context, 1, argv).ToLocalChecked());
}

void TemperatureWrapper::get_celsius(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  TemperatureSensor* obj = ObjectWrap::Unwrap<TemperatureWrapper>(
      args.Holder())->getObj();

  args.GetReturnValue().Set(Number::New(isolate, obj->get_celsius()));
}

void TemperatureWrapper::get_fahrenheit(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  TemperatureSensor* obj = ObjectWrap::Unwrap<TemperatureWrapper>(
      args.Holder())->getObj();

  args.GetReturnValue().Set(Number::New(isolate, obj->get_fahrenheit()));
}

// PROXIMITY DEVICE IMPLEMENTATION

ProximityWrapper::ProximityWrapper(ProximitySensor *obj) {
  m_sensor = obj;
}

ProximityWrapper::ProximityWrapper() {
  m_sensor = NULL;
}

ProximityWrapper::~ProximityWrapper() {
  ProximityWrapper::clean(m_sensor);
}

void ProximityWrapper::clean(void*elem) {
  ProximitySensor *sensor = reinterpret_cast<ProximitySensor*>(elem);
  if (sensor) {
    delete sensor;
  }
}

void ProximityWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  modal = FunctionTemplate::New(isolate, New);
  modal->Inherit(SensorDeviceWrapper::modal);
  modal->SetClassName(String::NewFromUtf8(isolate, "Proximity"));
  modal->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(modal, "get_presence", get_presence);
  constructor.Reset(isolate, modal->GetFunction());
  exports->Set(v8::String::NewFromUtf8(isolate, "Proximity"),
      modal->GetFunction());
}

void ProximityWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  int lenArg = 1;
  if (args.Length() != lenArg && args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  if (args.IsConstructCall()) {
    ProximityWrapper* obj = NULL;
    if (args.Length() == lenArg) {
      Local<External> ext = Local<External>::Cast(args[0]);
      obj = new ProximityWrapper(
          reinterpret_cast<ProximitySensor*>(ext->Value()));
    } else {
      obj = new ProximityWrapper();
    }
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

void ProximityWrapper::NewInstance(
    const v8::FunctionCallbackInfo<v8::Value>& args, ProximitySensor *value) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope scope(isolate);
  node::AtExit(clean, reinterpret_cast<void*>(value));
  Local<Value> argv[1] = {
    External::New(isolate, reinterpret_cast<void*>(value))
  };
  Local<Context> context = isolate->GetCurrentContext();
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  args.GetReturnValue().Set(
      cons->NewInstance(context, 1, argv).ToLocalChecked());
}

void ProximityWrapper::get_presence(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  ProximitySensor* obj = ObjectWrap::Unwrap<ProximityWrapper>(
      args.Holder())->getObj();

  args.GetReturnValue().Set(Number::New(isolate, obj->get_presence()));
}

// FLAME DEVICE IMPLEMENTATION

FlameWrapper::FlameWrapper(FlameSensor *obj) {
  m_sensor = obj;
}

FlameWrapper::FlameWrapper() {
  m_sensor = NULL;
}

FlameWrapper::~FlameWrapper() {
  FlameWrapper::clean(m_sensor);
}

void FlameWrapper::clean(void*elem) {
  FlameSensor *sensor = reinterpret_cast<FlameSensor*>(elem);
  if (sensor) {
    delete sensor;
  }
}

void FlameWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  modal = FunctionTemplate::New(isolate, New);
  modal->Inherit(SensorDeviceWrapper::modal);
  modal->SetClassName(String::NewFromUtf8(isolate, "Flame"));
  modal->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(modal, "get_signals", get_signals);
  constructor.Reset(isolate, modal->GetFunction());
  exports->Set(v8::String::NewFromUtf8(isolate, "Flame"),
      modal->GetFunction());
}

void FlameWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  int lenArg = 1;
  if (args.Length() != lenArg && args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  if (args.IsConstructCall()) {
    FlameWrapper* obj = NULL;
    if (args.Length() == lenArg) {
      Local<External> ext = Local<External>::Cast(args[0]);
      obj = new FlameWrapper(reinterpret_cast<FlameSensor*>(ext->Value()));
    } else {
      obj = new FlameWrapper();
    }
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

void FlameWrapper::NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args,
    FlameSensor *value) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope scope(isolate);
  node::AtExit(clean, reinterpret_cast<void*>(value));
  Local<Value> argv[1] = {
    External::New(isolate, reinterpret_cast<void*>(value))
  };
  Local<Context> context = isolate->GetCurrentContext();
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  args.GetReturnValue().Set(
      cons->NewInstance(context, 1, argv).ToLocalChecked());
}

void FlameWrapper::get_signals(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  FlameSensor* obj = ObjectWrap::Unwrap<FlameWrapper>(args.Holder())->getObj();

  args.GetReturnValue().Set(Number::New(isolate, obj->get_signals()));
}

// PRESSURE DEVICE IMPLEMENTATION

PressureWrapper::PressureWrapper(PressureSensor *obj) {
  m_sensor = obj;
}

PressureWrapper::PressureWrapper() {
  m_sensor = NULL;
}

PressureWrapper::~PressureWrapper() {
  PressureWrapper::clean(m_sensor);
}

void PressureWrapper::clean(void*elem) {
  PressureSensor *sensor = reinterpret_cast<PressureSensor*>(elem);
  if (sensor) {
    delete sensor;
  }
}

void PressureWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  modal = FunctionTemplate::New(isolate, New);
  modal->Inherit(SensorDeviceWrapper::modal);
  modal->SetClassName(String::NewFromUtf8(isolate, "Pressure"));
  modal->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(modal, "get_pressure", get_pressure);
  constructor.Reset(isolate, modal->GetFunction());
  exports->Set(v8::String::NewFromUtf8(isolate, "Pressure"),
      modal->GetFunction());
}


void PressureWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  int lenArg = 1;
  if (args.Length() != lenArg && args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  if (args.IsConstructCall()) {
    PressureWrapper* obj = NULL;
    if (args.Length() == lenArg) {
      Local<External> ext = Local<External>::Cast(args[0]);
      obj = new PressureWrapper(
          reinterpret_cast<PressureSensor*>(ext->Value()));
    } else {
      obj = new PressureWrapper();
    }
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

void PressureWrapper::NewInstance(
    const v8::FunctionCallbackInfo<v8::Value>& args, PressureSensor *value) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope scope(isolate);
  node::AtExit(clean, reinterpret_cast<void*>(value));
  Local<Value> argv[1] = {
    External::New(isolate, reinterpret_cast<void*>(value))
  };
  Local<Context> context = isolate->GetCurrentContext();
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  Local<Object> obj = cons->NewInstance(context, 1, argv).ToLocalChecked();
  args.GetReturnValue().Set(obj);
}

void PressureWrapper::get_pressure(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  PressureSensor* obj = ObjectWrap::Unwrap<PressureWrapper>(
      args.Holder())->getObj();

  args.GetReturnValue().Set(Number::New(isolate, obj->get_pressure()));
}

// HALL SENSOR DEVICE IMPLEMENTATION

HallWrapper::HallWrapper(HallSensor *obj) {
  m_sensor = obj;
}

HallWrapper::HallWrapper() {
  m_sensor = NULL;
}

HallWrapper::~HallWrapper() {
  HallWrapper::clean(m_sensor);
}

void HallWrapper::clean(void*elem) {
  HallSensor *sensor = reinterpret_cast<HallSensor*>(elem);
  if (sensor) {
    delete sensor;
  }
}

void HallWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  modal = FunctionTemplate::New(isolate, New);
  modal->Inherit(SensorDeviceWrapper::modal);
  modal->SetClassName(String::NewFromUtf8(isolate, "Hall"));
  modal->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(modal, "get_detection", get_detection);
  constructor.Reset(isolate, modal->GetFunction());
  exports->Set(v8::String::NewFromUtf8(isolate, "Hall"),
      modal->GetFunction());
}


void HallWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  int lenArg = 1;
  if (args.Length() != lenArg && args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  if (args.IsConstructCall()) {
    HallWrapper* obj = NULL;
    if (args.Length() == lenArg) {
      Local<External> ext = Local<External>::Cast(args[0]);
      obj = new HallWrapper(reinterpret_cast<HallSensor*>(ext->Value()));
    }  else {
      obj = new HallWrapper();
    }
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

void HallWrapper::NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args,
    HallSensor *value) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope scope(isolate);
  node::AtExit(clean, reinterpret_cast<void*>(value));
  Local<Value> argv[1] = {
    External::New(isolate, reinterpret_cast<void*>(value))
  };
  Local<Context> context = isolate->GetCurrentContext();
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  Local<Object> obj = cons->NewInstance(context, 1, argv).ToLocalChecked();
  args.GetReturnValue().Set(obj);
}

void HallWrapper::get_detection(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));

  HallSensor* obj = ObjectWrap::Unwrap<HallWrapper>(args.Holder())->getObj();

  args.GetReturnValue().Set(Number::New(isolate, obj->get_detection()));
}

// SENSOR MODULE IMPLEMENTATION

SensorWrapper::SensorWrapper() {
  m_sensor = new Sensor();
  node::AtExit(clean, reinterpret_cast<void*>(m_sensor));
}

SensorWrapper::~SensorWrapper() {
  SensorWrapper::clean(m_sensor);
}

void SensorWrapper::clean(void *elem) {
  Sensor *sensor = reinterpret_cast<Sensor*>(elem);
  if (sensor)
    delete sensor;
}

void SensorWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();
  Local<FunctionTemplate> modal = FunctionTemplate::New(isolate, New);

  modal->SetClassName(String::NewFromUtf8(isolate, "sensor"));
  modal->InstanceTemplate()->SetInternalFieldCount(1);
  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(modal, "list", list);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_sensor", get_sensor);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_accelerometer_sensor",
      get_accelerometer_sensor);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_humidity_sensor", get_humidity_sensor);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_light_sensor", get_light_sensor);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_temperature_sensor",
      get_temperature_sensor);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_proximity_sensor",
      get_proximity_sensor);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_flame_sensor", get_flame_sensor);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_pressure_sensor", get_pressure_sensor);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_gyro_sensor", get_gyro_sensor);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_hall_sensor", get_hall_sensor);
  constructor.Reset(isolate, modal->GetFunction());
  exports->Set(v8::String::NewFromUtf8(isolate, "sensor"),
      modal->GetFunction());
  SensorDeviceWrapper::Init(exports);
}

void SensorWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  if (args.IsConstructCall()) {
    try {
      SensorWrapper* obj = new SensorWrapper();
      obj->Wrap(args.This());
      args.GetReturnValue().Set(args.This());
    } catch(artik::ArtikException& e) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, e.what())));
    }
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    v8::MaybeLocal<Object> maybeObject = cons->NewInstance(context, 0, NULL);
    if (!maybeObject.IsEmpty())
      args.GetReturnValue().Set(maybeObject.ToLocalChecked());
  }
}

void SensorWrapper::list(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, "Wrong number of arguments")));
  Sensor* obj = ObjectWrap::Unwrap<SensorWrapper>(args.Holder())->getObj();

  try {
    SensorDeviceWrapper::NewArrayInstance(args, obj->list());
  } catch (artik::ArtikException e ) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, e.what())));
  }
}

void SensorWrapper::get_sensor(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 2)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  Sensor* obj = ObjectWrap::Unwrap<SensorWrapper>(args.Holder())->getObj();

  try {
    SensorDeviceWrapper::NewInstance(args,
        obj->get_sensor(args[0]->Int32Value(),
            (artik_sensor_device_t)args[1]->NumberValue()));
  } catch (artik::ArtikException e) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, e.what())));
  }
}

void SensorWrapper::get_accelerometer_sensor(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 1)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  Sensor* obj = ObjectWrap::Unwrap<SensorWrapper>(args.Holder())->getObj();

  try {
    AccelerometerWrapper::NewInstance(args,
        obj->get_accelerometer_sensor(args[0]->Int32Value()));
  } catch (artik::ArtikException e) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, e.what())));
  }
}

void SensorWrapper::get_humidity_sensor(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 1)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  Sensor* obj = ObjectWrap::Unwrap<SensorWrapper>(args.Holder())->getObj();

  try {
    HumidityWrapper::NewInstance(args,
        obj->get_humidity_sensor(args[0]->Int32Value()));
  } catch (artik::ArtikException e) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, e.what())));
  }
}

void SensorWrapper::get_light_sensor(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 1)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  Sensor* obj = ObjectWrap::Unwrap<SensorWrapper>(args.Holder())->getObj();

  try {
    LightWrapper::NewInstance(args,
        obj->get_light_sensor(args[0]->Int32Value()));
  } catch (artik::ArtikException e) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, e.what())));
  }
}

void SensorWrapper::get_temperature_sensor(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 1)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  Sensor* obj = ObjectWrap::Unwrap<SensorWrapper>(args.Holder())->getObj();

  try {
    TemperatureWrapper::NewInstance(args,
        obj->get_temperature_sensor(args[0]->Int32Value()));
  } catch (artik::ArtikException e) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, e.what())));
  }
}

void SensorWrapper::get_proximity_sensor(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 1)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  Sensor* obj = ObjectWrap::Unwrap<SensorWrapper>(args.Holder())->getObj();

  try {
    ProximityWrapper::NewInstance(args,
        obj->get_proximity_sensor(args[0]->Int32Value()));
  } catch (artik::ArtikException e) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, e.what())));
  }
}

void SensorWrapper::get_flame_sensor(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 1)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  Sensor* obj = ObjectWrap::Unwrap<SensorWrapper>(args.Holder())->getObj();

  try {
    FlameWrapper::NewInstance(args,
        obj->get_flame_sensor(args[0]->Int32Value()));
  } catch (artik::ArtikException e) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, e.what())));
  }
}

void SensorWrapper::get_pressure_sensor(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 1)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  Sensor* obj = ObjectWrap::Unwrap<SensorWrapper>(args.Holder())->getObj();

  try {
    PressureWrapper::NewInstance(args,
        obj->get_pressure_sensor(args[0]->Int32Value()));
  } catch (artik::ArtikException e) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, e.what())));
  }
}

void SensorWrapper::get_gyro_sensor(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 1)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  Sensor* obj = ObjectWrap::Unwrap<SensorWrapper>(args.Holder())->getObj();

  try {
    GyroWrapper::NewInstance(args, obj->get_gyro_sensor(args[0]->Int32Value()));
  } catch (artik::ArtikException e) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, e.what())));
  }
}

void SensorWrapper::get_hall_sensor(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 1)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  Sensor* obj = ObjectWrap::Unwrap<SensorWrapper>(args.Holder())->getObj();

  try {
    HallWrapper::NewInstance(args, obj->get_hall_sensor(args[0]->Int32Value()));
  } catch (artik::ArtikException e) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, e.what())));
  }
}

}  // namespace artik

