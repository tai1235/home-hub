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

#ifndef ADDON_MEDIA_MEDIA_H_
#define ADDON_MEDIA_MEDIA_H_

#include <node.h>
#include <node_object_wrap.h>

#include <uv.h>
#include <artik_media.hh>

#include <loop.h>

namespace artik {

class MediaWrapper : public node::ObjectWrap {
 public:
  static void Init(v8::Local<v8::Object> exports);

  Media* getObj() { return m_media; }
  v8::Persistent<v8::Function>* getFinishedCb() { return m_finished_cb; }

 private:
  MediaWrapper();
  ~MediaWrapper();

  static void New(const v8::FunctionCallbackInfo<v8::Value>& args);
  static v8::Persistent<v8::Function> constructor;

  static void play_sound_file(const v8::FunctionCallbackInfo<v8::Value>& args);

  Media* m_media;
  v8::Persistent<v8::Function>* m_finished_cb;
  GlibLoop* m_loop;
};

}  // namespace artik


#endif  // ADDON_MEDIA_MEDIA_H_
