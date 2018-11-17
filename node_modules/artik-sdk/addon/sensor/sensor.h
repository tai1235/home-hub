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

#ifndef ADDON_SENSOR_SENSOR_H_
#define ADDON_SENSOR_SENSOR_H_

#include <node.h>
#include <nan.h>
#include <node_object_wrap.h>

#include <uv.h>
#include <artik_sensor.hh>

#include <vector>

namespace artik {

class SensorDeviceWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object>);

  SensorDevice* getObj() { return m_sensor; }

  static void NewArrayInstance(const v8::FunctionCallbackInfo<v8::Value>& args,
      std::vector<SensorDevice*> const &);

  static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args,
      SensorDevice*);
  static v8::Handle<v8::FunctionTemplate> modal;

 private:
  explicit SensorDeviceWrapper(SensorDevice *obj);
  SensorDeviceWrapper();
  ~SensorDeviceWrapper();

  static void clean(void *obj);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void get_type(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_index(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_name(const v8::FunctionCallbackInfo<v8::Value>& args);

  SensorDevice *m_sensor;

  friend class AccelerometerWrapper;
  friend class HumidityWrapper;
  friend class LightWrapper;
  friend class TemperatureWrapper;
  friend class ProximityWrapper;
  friend class FlameWrapper;
  friend class PressureWrapper;
  friend class GyroWrapper;
  friend class HallWrapper;
};

class AccelerometerWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object>);

  AccelerometerSensor* getObj() { return m_sensor; }

  static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args,
      AccelerometerSensor*);

 private:
  explicit AccelerometerWrapper(AccelerometerSensor *obj);
  AccelerometerWrapper();
  ~AccelerometerWrapper();

  static void clean(void *obj);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;
  static v8::Handle<v8::FunctionTemplate> modal;

  static void get_speed_x(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_speed_y(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_speed_z(const v8::FunctionCallbackInfo<v8::Value>& args);

  AccelerometerSensor *m_sensor;
};

class GyroWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object>);

  GyroSensor* getObj() { return m_sensor; }

  static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args,
      GyroSensor*);

 private:
  explicit GyroWrapper(GyroSensor *obj);
  GyroWrapper();
  ~GyroWrapper();

  static void clean(void *obj);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;
  static v8::Handle<v8::FunctionTemplate> modal;

  static void get_yaw(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_roll(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_pitch(const v8::FunctionCallbackInfo<v8::Value>& args);

  GyroSensor *m_sensor;
};


class HumidityWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object>);

  HumiditySensor* getObj() { return m_sensor; }

  static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args,
      HumiditySensor*);

 private:
  explicit HumidityWrapper(HumiditySensor *obj);
  HumidityWrapper();
  ~HumidityWrapper();

  static void clean(void *obj);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;
  static v8::Handle<v8::FunctionTemplate> modal;

  static void get_humidity(const v8::FunctionCallbackInfo<v8::Value>& args);

  HumiditySensor *m_sensor;
};

class LightWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object>);

  LightSensor* getObj() { return m_sensor; }

  static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args,
      LightSensor*);

 private:
  explicit LightWrapper(LightSensor *obj);
  LightWrapper();
  ~LightWrapper();

  static void clean(void *obj);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;
  static v8::Handle<v8::FunctionTemplate> modal;

  static void get_intensity(const v8::FunctionCallbackInfo<v8::Value>& args);

  LightSensor *m_sensor;
};

class TemperatureWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object>);

  TemperatureSensor* getObj() { return m_sensor; }

  static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args,
      TemperatureSensor*);

 private:
  explicit TemperatureWrapper(TemperatureSensor *obj);
  TemperatureWrapper();
  ~TemperatureWrapper();

  static void clean(void *obj);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;
  static v8::Handle<v8::FunctionTemplate> modal;

  static void get_celsius(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_fahrenheit(const v8::FunctionCallbackInfo<v8::Value>& args);

  TemperatureSensor *m_sensor;
};

class ProximityWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object>);

  ProximitySensor* getObj() { return m_sensor; }

  static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args,
      ProximitySensor*);

 private:
  explicit ProximityWrapper(ProximitySensor *obj);
  ProximityWrapper();
  ~ProximityWrapper();

  static void clean(void *obj);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;
  static v8::Handle<v8::FunctionTemplate> modal;

  static void get_presence(const v8::FunctionCallbackInfo<v8::Value>& args);

  ProximitySensor *m_sensor;
};

class FlameWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object>);

  FlameSensor* getObj() { return m_sensor; }

  static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args,
      FlameSensor*);

 private:
  explicit FlameWrapper(FlameSensor *obj);
  FlameWrapper();
  ~FlameWrapper();

  static void clean(void *obj);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;
  static v8::Handle<v8::FunctionTemplate> modal;

  static void get_signals(const v8::FunctionCallbackInfo<v8::Value>& args);

  FlameSensor *m_sensor;
};

class PressureWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object>);

  PressureSensor* getObj() { return m_sensor; }

  static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args,
      PressureSensor*);

 private:
  explicit PressureWrapper(PressureSensor *obj);
  PressureWrapper();
  ~PressureWrapper();

  static void clean(void *obj);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;
  static v8::Handle<v8::FunctionTemplate> modal;

  static void get_pressure(const v8::FunctionCallbackInfo<v8::Value>& args);

  PressureSensor *m_sensor;
};

class HallWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object>);

  HallSensor* getObj() { return m_sensor; }

  static void NewInstance(const v8::FunctionCallbackInfo<v8::Value>& args,
      HallSensor*);

 private:
  explicit HallWrapper(HallSensor *obj);
  HallWrapper();
  ~HallWrapper();

  static void clean(void *obj);

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;
  static v8::Handle<v8::FunctionTemplate> modal;

  static void get_detection(const v8::FunctionCallbackInfo<v8::Value>& args);

  HallSensor *m_sensor;
};

class SensorWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object>);

  Sensor* getObj() { return m_sensor; }

 private:
  SensorWrapper();
  ~SensorWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void clean(void *obj);

  static void list(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_sensor(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_accelerometer_sensor(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_humidity_sensor(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_light_sensor(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_temperature_sensor(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_proximity_sensor(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_flame_sensor(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_pressure_sensor(
      const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_gyro_sensor(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void get_hall_sensor(const v8::FunctionCallbackInfo<v8::Value>& args);

  Sensor *m_sensor;
};

}  // namespace artik

#endif  // ADDON_SENSOR_SENSOR_H_

