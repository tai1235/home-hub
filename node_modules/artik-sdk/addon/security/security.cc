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

#include "security/security.h"
#include <artik_log.h>
#include <utils.h>
#include "base/ssl_config_converter.h"

namespace artik {

using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::Number;
using v8::Persistent;
using v8::String;
using v8::Value;
using v8::Handle;
using v8::Int32;
using v8::Boolean;
using v8::HandleScope;
using v8::Context;
using v8::Script;

Persistent<Function> SecurityWrapper::constructor;

static void security_cb_return(artik_error result,
  artik_time * signing_time_out,
  const v8::Persistent<v8::Function>& return_cb) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  char str[128] = {0};

  if (return_cb.IsEmpty())
    return;

  if (signing_time_out != NULL) {
    snprintf(str, sizeof(str),
    "new Date(Date.UTC(%d, %.2d, %.2d, %.2d, %.2d, %.2d))",
    signing_time_out->year, signing_time_out->month-1, signing_time_out->day,
    signing_time_out->hour, signing_time_out->minute, signing_time_out->second);

    Handle<String> src = String::NewFromUtf8(isolate,
      reinterpret_cast<char*>(str));
    Handle<Script> script = Script::Compile(src);
    Handle<Value> pkcs7_signing_date = script->Run();
    Handle<Value> argv[] = {
      Handle<Value>(v8::Integer::New(isolate, result)),
      Handle<Value>(String::NewFromUtf8(isolate, error_msg(result))),
      pkcs7_signing_date
    };

    Local<Function>::New(isolate, return_cb)->Call(
      isolate->GetCurrentContext()->Global(), 3, argv);
  } else {
    Handle<Value> argv[] = {
      Handle<Value>(v8::Integer::New(isolate, result)),
      Handle<Value>(String::NewFromUtf8(isolate, error_msg(result))),
    };

    Local<Function>::New(isolate, return_cb)->Call(
      isolate->GetCurrentContext()->Global(), 2, argv);
  }
}

SecurityWrapper::SecurityWrapper() {
  m_security = new Security();
}

SecurityWrapper::~SecurityWrapper() {
  delete m_security;
}

void SecurityWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();
  Local<FunctionTemplate> modal = FunctionTemplate::New(isolate, New);

  modal->SetClassName(String::NewFromUtf8(isolate, "security"));
  modal->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(modal, "get_certificate", get_certificate);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_ca_chain", get_ca_chain);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_key_from_cert", get_key_from_cert);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_random_bytes", get_random_bytes);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_certificate_sn", get_certificate_sn);
  NODE_SET_PROTOTYPE_METHOD(modal, "verify_signature_init",
    verify_signature_init);
  NODE_SET_PROTOTYPE_METHOD(modal, "verify_signature_update",
    verify_signature_update);
  NODE_SET_PROTOTYPE_METHOD(modal, "verify_signature_final",
    verify_signature_final);

  constructor.Reset(isolate, modal->GetFunction());
  exports->Set(v8::String::NewFromUtf8(isolate, "security"),
         modal->GetFunction());
}

void SecurityWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
                     isolate, "Wrong number of arguments")));
    return;
  }
  if (args.IsConstructCall()) {
    SecurityWrapper* obj = NULL;

    try {
      obj = new SecurityWrapper();
    } catch (artik::ArtikException &e) {
      isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, e.what())));
      return;
    }
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

void SecurityWrapper::get_certificate(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Security *obj = ObjectWrap::Unwrap<SecurityWrapper>(args.Holder())->getObj();
  char *cert = NULL;
  artik_error res = S_OK;

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  String::Utf8Value param0(args[0]->ToString());
  auto cert_id =
    to_artik_parameter<artik_security_certificate_id>(
      SSLConfigConverter::security_certificate_ids, *param0);
  if (!cert_id) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,
      "Wrong value of cert_id. ")));
    return;
  }

  try {
    res = obj->get_certificate(cert_id.value(), &cert);
    if (res != S_OK) {
      isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, error_msg(res))));
      return;
    }
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, cert));
    free(cert);
  } catch (artik::ArtikException &e) {
    isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, e.what())));
    return;
  }
}

void SecurityWrapper::get_ca_chain(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

    if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  Security *obj = ObjectWrap::Unwrap<SecurityWrapper>(args.Holder())->getObj();

  char *chain = NULL;
  artik_error res = S_OK;
  String::Utf8Value param0(args[0]->ToString());

  auto cert_id =
    to_artik_parameter<artik_security_certificate_id>(
      SSLConfigConverter::security_certificate_ids, *param0);
  if (!cert_id) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,
      "Wrong value of cert_id. ")));
    return;
  }

  res = obj->get_ca_chain(cert_id.value(), &chain);

  if (res != S_OK) {
    isolate->ThrowException(Exception::TypeError(
      String::NewFromUtf8(isolate, error_msg(res))));
    return;
  }

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, chain));
  free(chain);
}

void SecurityWrapper::get_key_from_cert(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
                     isolate, "Wrong number of arguments")));
    return;
  }

  Security *obj = ObjectWrap::Unwrap<SecurityWrapper>(args.Holder())->getObj();
  v8::String::Utf8Value param0(args[0]->ToString());
  char *cert = *param0;
  char *key = NULL;
  artik_error res = S_OK;

  try {
    res = obj->get_key_from_cert(cert, &key);
    if (res != S_OK) {
      isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, error_msg(res))));
      return;
    }
    args.GetReturnValue().Set(String::NewFromUtf8(isolate, key));
    free(key);
  } catch (artik::ArtikException &e) {
    isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, e.what())));
    return;
  }
}

void SecurityWrapper::get_random_bytes(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
                     isolate, "Wrong number of arguments")));
    return;
  }

  Security *obj = ObjectWrap::Unwrap<SecurityWrapper>(args.Holder())->getObj();

  int len = args[0]->NumberValue();
  unsigned char rand[len];
  artik_error res = S_OK;

  try {
    res = obj->get_random_bytes(rand, len);
    if (res != S_OK) {
      isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, error_msg(res))));
      return;
    }
    args.GetReturnValue().Set(
        Nan::CopyBuffer((const char*)rand, len).ToLocalChecked());
  } catch (artik::ArtikException &e) {
    isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, e.what())));
  }
}

void SecurityWrapper::get_certificate_sn(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  Security *obj = ObjectWrap::Unwrap<SecurityWrapper>(args.Holder())->getObj();

  unsigned int len = ARTIK_CERT_SN_MAXLEN;
  unsigned char sn[len];
  artik_error res = S_OK;
  String::Utf8Value param0(args[0]->ToString());
  auto cert_id =
    to_artik_parameter<artik_security_certificate_id>(
      SSLConfigConverter::security_certificate_ids, *param0);
  if (!cert_id) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,
      "Wrong value of cert_id. ")));
    return;
  }

  try {
    res = obj->get_certificate_sn(cert_id.value(), sn, &len);
    if (res != S_OK) {
      isolate->ThrowException(Exception::TypeError(
            String::NewFromUtf8(isolate, error_msg(res))));
      return;
    }
    args.GetReturnValue().Set(
        Nan::CopyBuffer(reinterpret_cast<char*>(sn), len).ToLocalChecked());
  } catch (artik::ArtikException &e) {
    isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, e.what())));
  }
}

void SecurityWrapper::verify_signature_init(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  v8::Persistent<v8::Function> return_cb;

  artik_error res = S_OK;
  char *signature_pem = NULL;
  char *root_ca = NULL;

  artik_time signing_time_in;
  artik_time signing_time_out;
  artik_time *time_in;

  SecurityWrapper *wrap = ObjectWrap::Unwrap<SecurityWrapper>(args.Holder());
  Security *obj = wrap->getObj();

  log_dbg("");

  if (!args[0]->IsString() || !args[1]->IsString()) {
    res = E_BAD_ARGS;
    args.GetReturnValue().Set(res);
    return;
  }

  if (args[3]->IsFunction())
    return_cb.Reset(isolate, Local<Function>::Cast(args[3]));

  String::Utf8Value param0(args[0]->ToString());
  signature_pem = *param0;
  if (!signature_pem) {
    res = E_BAD_ARGS;
    security_cb_return(res, NULL, return_cb);
    args.GetReturnValue().Set(res);
    return;
  }

  String::Utf8Value param1(args[1]->ToString());
  root_ca = *param1;
  if (!root_ca) {
    res = E_BAD_ARGS;
    security_cb_return(res, NULL, return_cb);
    args.GetReturnValue().Set(res);
    return;
  }

  if (args[2]->IsObject()) {
    Local<Value> tab_args[0] = NULL;
    Local<Object> object = Local<Object>::Cast(args[2]->ToObject());

    memset(&signing_time_in, 0, sizeof(signing_time_in));
    signing_time_in.second = Local<Function>::Cast(object->Get(
      String::NewFromUtf8(isolate, "getUTCSeconds")))->Call(
      object, 0, tab_args)->Int32Value();
    signing_time_in.minute = Local<Function>::Cast(object->Get(
      String::NewFromUtf8(isolate, "getUTCMinutes")))->Call(
    object, 0, tab_args)->Int32Value();
    signing_time_in.hour = Local<Function>::Cast(object->Get(
      String::NewFromUtf8(isolate, "getUTCHours")))->Call(
      object, 0, tab_args)->Int32Value();
    signing_time_in.day = Local<Function>::Cast(object->Get(
      String::NewFromUtf8(isolate, "getUTCDate")))->Call(
      object, 0, tab_args)->Int32Value();
    signing_time_in.month = Local<Function>::Cast(object->Get(
      String::NewFromUtf8(isolate, "getUTCMonth")))->Call(
      object, 0, tab_args)->Int32Value() + 1;
    signing_time_in.year = Local<Function>::Cast(object->Get(
      String::NewFromUtf8(isolate, "getUTCFullYear")))->Call(
      object, 0, tab_args)->Int32Value();
    signing_time_in.day_of_week = Local<Function>::Cast(object->Get(
      String::NewFromUtf8(isolate, "getUTCDay")))->Call(
      object, 0, tab_args)->Int32Value();
    signing_time_in.msecond = Local<Function>::Cast(object->Get(
      String::NewFromUtf8(isolate, "getUTCMilliseconds")))->Call(
      object, 0, tab_args)->Int32Value();
    time_in = &signing_time_in;
  } else {
    time_in = NULL;
  }

  res = obj->verify_signature_init(signature_pem, root_ca,
    time_in, &signing_time_out);

  security_cb_return(res, &signing_time_out, return_cb);

  args.GetReturnValue().Set(res);
}

void SecurityWrapper::verify_signature_update(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  artik_error res = S_OK;
  unsigned char *data = NULL;
  unsigned int data_len = 0;
  v8::Persistent<v8::Function> return_cb;

  SecurityWrapper *wrap =
    ObjectWrap::Unwrap<SecurityWrapper>(args.Holder());
  Security *obj = wrap->getObj();

  log_dbg("");

  if (!node::Buffer::HasInstance(args[0])) {
    res = E_BAD_ARGS;
    args.GetReturnValue().Set(res);
    return;
  }
  if (args[1]->IsFunction())
    return_cb.Reset(isolate, Local<Function>::Cast(args[1]));

  data = reinterpret_cast<unsigned char *>(node::Buffer::Data(args[0]));
  data_len = node::Buffer::Length(args[0]);

  res = obj->verify_signature_update(data, data_len);
  security_cb_return(res, NULL, return_cb);

  args.GetReturnValue().Set(res);
}

void SecurityWrapper::verify_signature_final(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  artik_error res = S_OK;
  v8::Persistent<v8::Function> return_cb;

  SecurityWrapper *wrap =
    ObjectWrap::Unwrap<SecurityWrapper>(args.Holder());
  Security *obj = wrap->getObj();

  log_dbg("");

  if (args[0]->IsFunction())
    return_cb.Reset(isolate, Local<Function>::Cast(args[0]));

  res = obj->verify_signature_final();

  security_cb_return(res, NULL, return_cb);

  args.GetReturnValue().Set(res);
}

}  // namespace artik
