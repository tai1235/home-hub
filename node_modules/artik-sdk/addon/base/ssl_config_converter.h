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


#ifndef ADDON_BASE_SSL_CONFIG_CONVERTER_H_
#define ADDON_BASE_SSL_CONFIG_CONVERTER_H_

#include <artik_ssl.h>

#include <node.h>
#include <node_object_wrap.h>

#include <memory>

namespace artik {
struct SSLConfigConverter {
  static std::unique_ptr<artik_ssl_config> convert(
    v8::Isolate *isolate,
    v8::Local<v8::Value> val);

  static std::array<const char*, 2> security_certificate_ids;
};
}  // namespace artik

#endif  // ADDON_BASE_SSL_CONFIG_CONVERTER_H_
