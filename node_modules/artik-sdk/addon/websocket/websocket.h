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

#ifndef ADDON_WEBSOCKET_WEBSOCKET_H_
#define ADDON_WEBSOCKET_WEBSOCKET_H_

#include <node.h>
#include <node_object_wrap.h>

#include <uv.h>
#include <artik_websocket.hh>

#include <utils.h>
#include <loop.h>

namespace artik {

class WebsocketWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  Websocket* getObj() { return m_websocket; }
  v8::Persistent<v8::Function>* getConnectionCb() { return m_connection_cb; }
  v8::Persistent<v8::Function>* getReceiveCb() { return m_receive_cb; }

 private:
  WebsocketWrapper();
  explicit WebsocketWrapper(char* uri, artik_ssl_config *ssl_config);
  ~WebsocketWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;
  static void open_stream(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void write_stream(const v8::FunctionCallbackInfo<v8::Value>& args);
  static void close_stream(const v8::FunctionCallbackInfo<v8::Value>& args);

  Websocket* m_websocket;
  v8::Persistent<v8::Function>* m_connection_cb;
  v8::Persistent<v8::Function>* m_receive_cb;
  GlibLoop* m_loop;
};

}  // namespace artik

#endif  // ADDON_WEBSOCKET_WEBSOCKET_H_
