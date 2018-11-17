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

#include "base/ssl_config_converter.h"

#include <artik_log.h>
#include <utils.h>

#include <string>

static std::array<const char*, 3> ssl_verifies = {
  "none",
  "optional",
  "required" };

namespace artik {

using v8::Exception;
using v8::Isolate;
using v8::Local;
using v8::String;
using v8::Value;

std::array<const char*, 2> SSLConfigConverter::security_certificate_ids = {
  "artik",
  "manufacturer" };

std::unique_ptr<artik_ssl_config> SSLConfigConverter::convert(
  Isolate *isolate, Local<Value> val) {
  std::unique_ptr<artik_ssl_config> ssl_config(new artik_ssl_config);

  memset(ssl_config.get(), 0, sizeof(artik_ssl_config));

  /* se_config parameter */
  log_dbg("Parse ssl_config");
  auto se_config = js_object_attribute_to_cpp<Local<Value>>(val, "se_config");
  if (se_config) {
    log_dbg("Parse SE config");
    auto cert_id_str =
      js_object_attribute_to_cpp<std::string>(se_config.value(),
                                              "certificate_identifier");
    if (!cert_id_str) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,
        "Wrong definition of se_config:"
        " certificate_identifier is not a string.")));
      return nullptr;
    }

    auto cert_id =
      to_artik_parameter<artik_security_certificate_id>(
        security_certificate_ids,
        cert_id_str.value().c_str());

    if (!cert_id) {
      std::string msg = "Wrong definition of se_config: Unknow value "
                      + cert_id_str.value()
                      + " for attribute certificate_identifier";
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
                     isolate, msg.c_str())));
      return nullptr;
    }

    log_dbg("enable secure element");
    ssl_config->se_config.use_se = true;
    ssl_config->se_config.certificate_id = cert_id.value();
  }

  log_dbg("ssl_config->se_config.use_se = %d", ssl_config->se_config.use_se);
  /* ca_cert parameter */
  auto ca_cert = js_object_attribute_to_cpp<Local<Value>>(val, "ca_cert");

  if (ca_cert) {
    if (!node::Buffer::HasInstance(ca_cert.value())) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,
        "Wrong definition of ca_cert")));
      return nullptr;
    }

    char *val = reinterpret_cast<char *>(node::Buffer::Data(ca_cert.value()));
    size_t len = node::Buffer::Length(ca_cert.value());

    ssl_config->ca_cert.data = strdup(val);
    ssl_config->ca_cert.len = len;
  }

  /* client_cert parameter */
  auto client_cert = js_object_attribute_to_cpp<Local<Value>>(val,
    "client_cert");

  if (client_cert) {
    if (!node::Buffer::HasInstance(client_cert.value())) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,
        "Wrong definition of client_cert")));
      return nullptr;
    }
    char *val = reinterpret_cast<char *>(node::Buffer::Data(
      client_cert.value()));
    size_t len = node::Buffer::Length(client_cert.value());

    ssl_config->client_cert.data = strdup(val);
    ssl_config->client_cert.len = len;
  }

  /* client_key parameter */
  auto client_key = js_object_attribute_to_cpp<Local<Value>>(val, "client_key");

  if (client_key) {
    if (!node::Buffer::HasInstance(client_key.value())) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,
        "Wrong definition of client_key")));
      return nullptr;
    }

    char *val = reinterpret_cast<char *>(node::Buffer::Data(
      client_key.value()));
    size_t len = node::Buffer::Length(client_key.value());

    ssl_config->client_key.data = strdup(val);
    ssl_config->client_key.len = len;
  }

  /* verify_cert parameter */
  auto verify_cert_str = js_object_attribute_to_cpp<std::string>(
    val,
    "verify_cert");
  if (verify_cert_str) {
    auto verify_cert =
      to_artik_parameter<artik_ssl_verify_t>(
        ssl_verifies,
        verify_cert_str.value().c_str());

    if (!verify_cert) {
      isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,
        "Wrong value for verify_cert : expect 'none',"
        "'optional' or 'required'.")));
      return nullptr;
    }

    ssl_config->verify_cert = verify_cert.value();
  }

  return ssl_config;
}

}  // namespace artik
