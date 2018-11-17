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

#include "utils.h"

#include <artik_log.h>

namespace artik {

namespace details {
  Optional<std::string> converter<std::string>::js_type_to_cpp(
    v8::Local<v8::Value> val) {
    Nan::MaybeLocal<v8::String> maybe_str = Nan::To<v8::String>(val);
    v8::Local<v8::String> str;
    if (!maybe_str.ToLocal(&str))
      return Optional<std::string>();

    Nan::Utf8String str_val(str);
    return Optional<std::string>(std::string(*str_val));
  }

  Optional<v8::Local<v8::Array>>
  converter<v8::Local<v8::Array>>::js_type_to_cpp(v8::Local<v8::Value> val) {
    if (!val->IsArray())
      return Optional<v8::Local<v8::Array>>();

    return val.As<v8::Array>();
  }

  Optional<v8::Local<v8::Function>>
  converter<v8::Local<v8::Function>>::js_type_to_cpp(v8::Local<v8::Value> val) {
    if (!val->IsFunction())
      return Optional<v8::Local<v8::Function>>();

    return val.As<v8::Function>();
  }
}  // namespace details

template<> Optional<v8::Local<v8::Value>>
js_object_attribute_to_cpp<v8::Local<v8::Value>>(v8::Local<v8::Value> obj,
    const std::string& field) {
  v8::Isolate* isolate = v8::Isolate::GetCurrent();
  Nan::MaybeLocal<v8::Object> maybe_object = Nan::To<v8::Object>(obj);
  v8::Local<v8::Object> object;
  if (!maybe_object.ToLocal(&object))
    return Optional<v8::Local<v8::Value>>();

  log_dbg("obj is object");

  v8::MaybeLocal<v8::Value> maybe_val = object->Get(
    isolate->GetCurrentContext(), Nan::New(field).ToLocalChecked());
  v8::Local<v8::Value> val;
  if (!maybe_val.ToLocal(&val)) {
    log_dbg("field not found");
    return Optional<v8::Local<v8::Value>>();
  }

  if (val->IsNull() || val->IsUndefined()) {
    return Optional<v8::Local<v8::Value>>();
  }

  log_dbg("obj is null %d", val->IsNull());
  log_dbg("obj is undefined %d", val->IsUndefined());
  log_dbg("obj is found");

  return Optional<v8::Local<v8::Value>>(val);
}
}  // namespace artik
