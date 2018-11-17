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

#include "http/http.h"

#include <unistd.h>
#include <node_buffer.h>
#include <artik_log.h>

#include <memory>
#include <string>

#include "base/ssl_config_converter.h"

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
using v8::Handle;
using v8::Int32;
using v8::Context;

Persistent<Function> HttpWrapper::constructor;

static int on_http_data(char *data, unsigned int len, void *user_data) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  HttpWrapper* wrap = reinterpret_cast<HttpWrapper*>(user_data);

  log_dbg("");

  if (!wrap->getDataCb())
    return 0;

  Handle<Value> argv[] = {
    Handle<Value>(Nan::CopyBuffer(data, len).ToLocalChecked())
  };

  Local<Function>::New(isolate, *wrap->getDataCb())->Call(
    isolate->GetCurrentContext()->Global(), 1, argv);

  return len;
}

static void on_http_error(artik_error result, int status, char * response,
  void *user_data) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  HttpWrapper* wrap = reinterpret_cast<HttpWrapper*>(user_data);

  log_dbg("");

  if (!wrap->getErrorCb())
    return;

  Handle<Value> argv[] = {
    Handle<Value>(String::NewFromUtf8(isolate, error_msg(result))),
    Handle<Value>(v8::Integer::New(isolate, status)),
  };

  Local<Function>::New(isolate, *wrap->getErrorCb())->Call(
    isolate->GetCurrentContext()->Global(), 2, argv);
}

static void http_response_get_callback(artik_error result, int status,
  char *response, void *user_data) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  HttpWrapper* wrap = reinterpret_cast<HttpWrapper*>(user_data);

  log_dbg("");

  if (!wrap->getResponseGetCb())
    return;

  Handle<Value> argv[] = {
    Handle<Value>(String::NewFromUtf8(isolate, result != S_OK ?
                                                error_msg(result) : response)),
    Handle<Value>(v8::Integer::New(isolate, status)),
  };

  Local<Function>::New(isolate, *wrap->getResponseGetCb())->Call(
    isolate->GetCurrentContext()->Global(), 2, argv);
}

static void http_response_post_callback(artik_error result, int status,
  char *response, void *user_data) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  HttpWrapper* wrap = reinterpret_cast<HttpWrapper*>(user_data);

  log_dbg("");

  if (!wrap->getResponsePostCb())
    return;

  Handle<Value> argv[] = {
    Handle<Value>(String::NewFromUtf8(isolate, result != S_OK ?
                                                error_msg(result) : response)),
    Handle<Value>(v8::Integer::New(isolate, status)),
  };

  Local<Function>::New(isolate, *wrap->getResponsePostCb())->Call(
    isolate->GetCurrentContext()->Global(), 2, argv);
}

static void http_response_put_callback(artik_error result, int status,
  char *response, void *user_data) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  HttpWrapper* wrap = reinterpret_cast<HttpWrapper*>(user_data);

  log_dbg("");

  if (!wrap->getResponsePutCb())
    return;

  Handle<Value> argv[] = {
    Handle<Value>(String::NewFromUtf8(isolate, result != S_OK ?
                                                error_msg(result) : response)),
    Handle<Value>(v8::Integer::New(isolate, status)),
  };

  Local<Function>::New(isolate, *wrap->getResponsePutCb())->Call(
    isolate->GetCurrentContext()->Global(), 2, argv);
}

static void http_response_del_callback(artik_error result, int status,
  char *response, void *user_data) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  HttpWrapper* wrap = reinterpret_cast<HttpWrapper*>(user_data);

  log_dbg("");

  if (!wrap->getResponseDelCb())
    return;

  Handle<Value> argv[] = {
    Handle<Value>(String::NewFromUtf8(isolate, result != S_OK ?
                                                error_msg(result) : response)),
    Handle<Value>(v8::Integer::New(isolate, status)),
  };

  Local<Function>::New(isolate, *wrap->getResponseDelCb())->Call(
    isolate->GetCurrentContext()->Global(), 2, argv);
}

HttpWrapper::HttpWrapper() {
  m_http = new Http();
  m_loop = GlibLoop::Instance();
  m_loop->attach();
}

HttpWrapper::~HttpWrapper() {
  m_loop->detach();
  delete m_http;
}

void HttpWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "http"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_stream", get_stream);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get", get);
  NODE_SET_PROTOTYPE_METHOD(tpl, "post", post);
  NODE_SET_PROTOTYPE_METHOD(tpl, "put", put);
  NODE_SET_PROTOTYPE_METHOD(tpl, "del", del);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "http"),
      tpl->GetFunction());
}

void HttpWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  log_dbg("Create Http JS Wrapper");
  if (args.IsConstructCall()) {
    HttpWrapper* obj = new HttpWrapper();
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

void HttpWrapper::get_stream(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HttpWrapper* obj = ObjectWrap::Unwrap<HttpWrapper>(args.Holder());
  Http* http = obj->getObj();
  artik_http_headers* headers = NULL;
  std::unique_ptr<artik_ssl_config> ssl_config(nullptr);

  log_dbg("");

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  /* Copy headers */
  if (args[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(args[1]);

    if (array->Length() > 2) {
      int num_headers = array->Length()/2;
      artik_http_header_field* fields =
        reinterpret_cast<artik_http_header_field*>(malloc(
        num_headers*sizeof(artik_http_header_field)));
      if (!fields) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Memory allocation error")));
        return;
      }

      for (int i=0; i < num_headers; i++) {
        v8::String::Utf8Value name(array->Get(i*2)->ToString());
        v8::String::Utf8Value data(array->Get((i*2)+1)->ToString());

        fields[i].name = strndup(*name, MAX_HEADER_SIZE);
        fields[i].data = strndup(*data, MAX_HEADER_SIZE);
      }

      headers = reinterpret_cast<artik_http_headers*>(
        malloc(sizeof(artik_http_headers)));
      if (!headers) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Memory allocation error")));
        return;
      }

      headers->num_fields = num_headers;
      headers->fields = fields;
    }
  }

  /* SSL Configuration */
  if (args[2]->IsObject()) {
    ssl_config = SSLConfigConverter::convert(isolate, args[2]);
    if (!ssl_config) {
      return;
    }
  }

  /* Data Callback */
  if (args[3]->IsFunction()) {
    obj->m_data_cb = new v8::Persistent<v8::Function>();
    obj->m_data_cb->Reset(isolate, Local<Function>::Cast(args[3]));
  }

  /* Error Callback */
  if (args[4]->IsFunction()) {
    obj->m_error_cb = new v8::Persistent<v8::Function>();
    obj->m_error_cb->Reset(isolate, Local<Function>::Cast(args[4]));
  }

  v8::String::Utf8Value param0(args[0]->ToString());
  const char *url = *param0;
  artik_error ret = http->get_stream_async(url, headers, on_http_data,
    on_http_error, reinterpret_cast<void*>(obj), ssl_config.get());

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, error_msg(ret)));
}

void HttpWrapper::get(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HttpWrapper* obj = ObjectWrap::Unwrap<HttpWrapper>(args.Holder());
  Http* http = obj->getObj();
  artik_http_headers* headers = NULL;
  std::unique_ptr<artik_ssl_config> ssl_config(nullptr);

  log_dbg("");

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  /* Copy headers */
  if (args[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(args[1]);

    if (array->Length() > 2) {
      int num_headers = array->Length() / 2;
      artik_http_header_field *fields =
          reinterpret_cast<artik_http_header_field*>(malloc(
          num_headers * sizeof(artik_http_header_field)));
      if (!fields) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Memory allocation error")));
        return;
      }

      for (int i = 0; i < num_headers; i++) {
        v8::String::Utf8Value name(array->Get(i*2)->ToString());
        v8::String::Utf8Value data(array->Get((i*2)+1)->ToString());

        fields[i].name = strndup(*name, MAX_HEADER_SIZE);
        fields[i].data = strndup(*data, MAX_HEADER_SIZE);
      }

      headers = reinterpret_cast<artik_http_headers*>(malloc(
          sizeof(artik_http_headers)));
      if (!headers) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Memory allocation error")));
        return;
      }

      headers->num_fields = num_headers;
      headers->fields = fields;
    }
  }

  /* SSL Configuration */
  if (args[2]->IsObject()) {
    ssl_config = SSLConfigConverter::convert(isolate, args[2]);
    if (!ssl_config) {
      return;
    }
  }

  /* If callback is provided, make the call asynchronous */
  if (args[3]->IsFunction()) {
    obj->m_response_get_cb = new v8::Persistent<v8::Function>();
    obj->m_response_get_cb->Reset(isolate, Local<Function>::Cast(args[3]));

    v8::String::Utf8Value param0(args[0]->ToString());
    const char *url = *param0;
    artik_error ret = http->get_async(
      url, headers, http_response_get_callback,
      reinterpret_cast<void*>(obj), ssl_config.get());

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, error_msg(ret)));

  } else { /* Otherwise make the call directly */
    v8::String::Utf8Value param0(args[0]->ToString());
    const char *url = *param0;
    char *response = NULL;
    artik_error ret =
      http->get(url, headers, &response, NULL, ssl_config.get());

    if (ret != S_OK)
      response = strndup(error_msg(ret), MAX_ERRR_MSG_LEN);

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

void HttpWrapper::post(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HttpWrapper* obj = ObjectWrap::Unwrap<HttpWrapper>(args.Holder());
  Http* http = obj->getObj();
  artik_http_headers* headers = NULL;
  std::unique_ptr<artik_ssl_config> ssl_config(nullptr);

  log_dbg("");

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  /* Copy headers */
  if (args[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(args[1]);

    if (array->Length() > 2) {
      int num_headers = array->Length()/2;
      artik_http_header_field* fields =
          reinterpret_cast<artik_http_header_field*>(malloc(
          num_headers * sizeof(artik_http_header_field)));
      if (!fields) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Memory allocation error")));
        return;
      }

      for (int i = 0; i < num_headers; i++) {
        v8::String::Utf8Value name(array->Get(i*2)->ToString());
        v8::String::Utf8Value data(array->Get((i*2)+1)->ToString());

        fields[i].name = strndup(*name, MAX_HEADER_SIZE);
        fields[i].data = strndup(*data, MAX_HEADER_SIZE);
      }

      headers = reinterpret_cast<artik_http_headers*>(malloc(
          sizeof(artik_http_headers)));
      if (!headers) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Memory allocation error")));
        return;
      }

      headers->num_fields = num_headers;
      headers->fields = fields;
    }
  }

  /* SSL Configuration */
  if (args[3]->IsObject()) {
    ssl_config = SSLConfigConverter::convert(isolate, args[3]);
    if (!ssl_config) {
      return;
    }
  }

  /* If callback is provided, make the call asynchronous */
  if (args[4]->IsFunction()) {
    obj->m_response_post_cb = new v8::Persistent<v8::Function>();
    obj->m_response_post_cb->Reset(isolate, Local<Function>::Cast(args[4]));

    v8::String::Utf8Value param0(args[0]->ToString());
    const char *url = *param0;
    const char *body = NULL;

    /* copy body data if provided */
    if (args[2]->IsString()) {
      v8::String::Utf8Value param2(args[2]->ToString());
      body = strndup(*param2, strlen(*param2));
    }

    artik_error ret = http->post_async(
      url, headers, body,
      http_response_post_callback,
      reinterpret_cast<void*>(obj), ssl_config.get());

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, error_msg(ret)));

  } else { /* Otherwise make the call directly */
    v8::String::Utf8Value param0(args[0]->ToString());
    const char *url = *param0;
    const char *body = NULL;
    char *response = NULL;
    artik_error ret = S_OK;

    /* copy body data if provided */
    if (args[2]->IsString()) {
      v8::String::Utf8Value param2(args[2]->ToString());
      body = strndup(*param2, strlen(*param2));
    }

    ret = http->post(url, headers, body, &response, NULL, ssl_config.get());

    if (ret != S_OK)
      response = strndup(error_msg(ret), MAX_ERRR_MSG_LEN);

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

void HttpWrapper::put(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HttpWrapper* obj = ObjectWrap::Unwrap<HttpWrapper>(args.Holder());
  Http* http = obj->getObj();
  artik_http_headers* headers = NULL;
  std::unique_ptr<artik_ssl_config> ssl_config(nullptr);

  log_dbg("");

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  /* Copy headers */
  if (args[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(args[1]);

    if (array->Length() > 2) {
      int num_headers = array->Length()/2;
      artik_http_header_field* fields =
          reinterpret_cast<artik_http_header_field*>(malloc(
          num_headers * sizeof(artik_http_header_field)));
      if (!fields) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Memory allocation error")));
        return;
      }

      for (int i = 0; i < num_headers; i++) {
        v8::String::Utf8Value name(array->Get(i*2)->ToString());
        v8::String::Utf8Value data(array->Get((i*2)+1)->ToString());

        fields[i].name = strndup(*name, MAX_HEADER_SIZE);
        fields[i].data = strndup(*data, MAX_HEADER_SIZE);
      }

      headers = reinterpret_cast<artik_http_headers*>(malloc(
          sizeof(artik_http_headers)));
      if (!headers) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Memory allocation error")));
        return;
      }

      headers->num_fields = num_headers;
      headers->fields = fields;
    }
  }

  /* SSL Configuration */
  if (args[3]->IsObject()) {
    ssl_config = SSLConfigConverter::convert(isolate, args[3]);
    if (!ssl_config) {
      return;
    }
  }

  /* If callback is provided, make the call asynchronous */
  if (args[4]->IsFunction()) {
    obj->m_response_put_cb = new v8::Persistent<v8::Function>();
    obj->m_response_put_cb->Reset(isolate, Local<Function>::Cast(args[4]));

    v8::String::Utf8Value param0(args[0]->ToString());
    const char *url = *param0;
    const char *body = NULL;

    /* copy body data if provided */
    if (args[2]->IsString()) {
      v8::String::Utf8Value param2(args[2]->ToString());
      body = strndup(*param2, strlen(*param2));
    }

    artik_error ret = http->put_async(
      url, headers, body,
      http_response_put_callback,
      reinterpret_cast<void*>(obj), ssl_config.get());

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, error_msg(ret)));

  } else { /* Otherwise make the call directly */
    v8::String::Utf8Value param0(args[0]->ToString());
    const char *url = *param0;
    const char *body = NULL;
    char *response = NULL;
    artik_error ret = S_OK;

    /* copy body data if provided */
    if (args[2]->IsString()) {
      v8::String::Utf8Value param2(args[2]->ToString());
      body = strndup(*param2, strlen(*param2));
    }

    ret = http->put(url, headers, body, &response, NULL, ssl_config.get());

    if (ret != S_OK)
      response = strndup(error_msg(ret), MAX_ERRR_MSG_LEN);

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

void HttpWrapper::del(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  HttpWrapper* obj = ObjectWrap::Unwrap<HttpWrapper>(args.Holder());
  Http* http = obj->getObj();
  artik_http_headers* headers = NULL;
  std::unique_ptr<artik_ssl_config> ssl_config(nullptr);

  log_dbg("");

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  /* Copy headers */
  if (args[1]->IsArray()) {
    Local<Array> array = Local<Array>::Cast(args[1]);

    if (array->Length() > 2) {
      int num_headers = array->Length()/2;
      artik_http_header_field *fields =
          reinterpret_cast<artik_http_header_field*>(malloc(
          num_headers * sizeof(artik_http_header_field)));
      if (!fields) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Memory allocation error")));
        return;
      }

      for (int i = 0; i < num_headers; i++) {
        v8::String::Utf8Value name(array->Get(i*2)->ToString());
        v8::String::Utf8Value data(array->Get((i*2)+1)->ToString());

        fields[i].name = strndup(*name, MAX_HEADER_SIZE);
        fields[i].data = strndup(*data, MAX_HEADER_SIZE);
      }

      headers = reinterpret_cast<artik_http_headers*>(malloc(
          sizeof(artik_http_headers)));
      if (!headers) {
        isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
          isolate, "Memory allocation error")));
        return;
      }

      headers->num_fields = num_headers;
      headers->fields = fields;
    }
  }

  /* SSL Configuration */
  if (args[2]->IsObject()) {
    ssl_config = SSLConfigConverter::convert(isolate, args[2]);
    if (!ssl_config) {
      return;
    }
  }

  /* If callback is provided, make the call asynchronous */
  if (args[3]->IsFunction()) {
    obj->m_response_del_cb = new v8::Persistent<v8::Function>();
    obj->m_response_del_cb->Reset(isolate, Local<Function>::Cast(args[3]));

    v8::String::Utf8Value param0(args[0]->ToString());
    const char *url = *param0;
    artik_error ret =
      http->del_async(url, headers, http_response_del_callback,
                      reinterpret_cast<void*>(obj), ssl_config.get());

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, error_msg(ret)));

  } else { /* Otherwise make the call directly */
    v8::String::Utf8Value param0(args[0]->ToString());
    const char *url = *param0;
    char *response = NULL;
    artik_error ret = S_OK;

    ret = http->del(url, headers, &response, NULL, ssl_config.get());

    if (ret != S_OK)
      response = strndup(error_msg(ret), MAX_ERRR_MSG_LEN);

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

}  // namespace artik
