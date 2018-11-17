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

#ifndef ADDON_MQTT_MQTT_H_
#define ADDON_MQTT_MQTT_H_

#include <node.h>
#include <node_object_wrap.h>

#include <uv.h>
#include <artik_mqtt.hh>
#include <utils.h>

#include <loop.h>

using v8::Function;
using v8::Local;

namespace artik {

class MqttWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  Mqtt* getObj() { return m_mqtt; }
  v8::Persistent<v8::Function>* getConnectCb() { return m_connect_cb; }
  v8::Persistent<v8::Function>* getDisconnectCb() { return m_disconnect_cb; }
  v8::Persistent<v8::Function>* getPublishCb() { return m_publish_cb; }
  v8::Persistent<v8::Function>* getMessageCb() { return m_message_cb; }
  v8::Persistent<v8::Function>* getSubscribeCb() { return m_subscribe_cb; }
  v8::Persistent<v8::Function>* getUnsubscribeCb() { return m_unsubscribe_cb; }

 private:
  explicit MqttWrapper(artik_mqtt_config const &);
  ~MqttWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void set_willmsg(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void free_willmsg(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void clear_willmsg(const v8::FunctionCallbackInfo<v8::Value>& args);

  static void connect(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void disconnect(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void subscribe(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void unsubscribe(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void publish(const v8::FunctionCallbackInfo<v8::Value>& args);

  Mqtt *m_mqtt;
  v8::Persistent<v8::Function>* m_connect_cb;
  v8::Persistent<v8::Function>* m_disconnect_cb;
  v8::Persistent<v8::Function>* m_publish_cb;
  v8::Persistent<v8::Function>* m_message_cb;
  v8::Persistent<v8::Function>* m_subscribe_cb;
  v8::Persistent<v8::Function>* m_unsubscribe_cb;
  GlibLoop* m_loop;
};

}  // namespace artik


#endif  // ADDON_MQTT_MQTT_H_
