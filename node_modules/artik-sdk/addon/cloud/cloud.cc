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

#include "cloud/cloud.h"

#include <unistd.h>
#include <artik_log.h>

#include <string>
#include <utility>

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
using v8::MaybeLocal;
using v8::JSON;

Persistent<Function> CloudWrapper::constructor;

static void cloud_callback(artik_error ret, char *response, void *user_data) {
  Isolate *isolate = Isolate::GetCurrent();
  v8::HandleScope scope(isolate);
  CloudWork *work = static_cast<CloudWork*>(user_data);
  Local<Value> error = Nan::Null();
  Local<Value> val = Nan::Null();

  log_dbg("");

  if (!work) {
    return;
  }

  if (ret != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(ret));
    error = Nan::New<String>(msg).ToLocalChecked();
  } else {
    MaybeLocal<Value> result =
      JSON::Parse(isolate, Nan::New<String>(response).ToLocalChecked());
    if (!result.IsEmpty()) {
      val = result.ToLocalChecked();
    } else {
      error = Nan::New<String>("Error: JSON Parser error").ToLocalChecked();
    }
  }

  Handle<Value> argv[] = {
    error,
    val
  };

  Nan::Call(work->callback, 2, argv);

  delete work;
}

static void on_receive_callback(void *user_data, void *result) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  CloudWrapper *obj = reinterpret_cast<CloudWrapper *>(user_data);

  log_dbg("");

  if (!obj || !obj->getReceiveCb())
    return;

  Handle<Value> argv[] = {
    Handle<Value>(String::NewFromUtf8(isolate,
        result ? reinterpret_cast<char*>(result) : "null")),
  };

  Local<Function>::New(isolate, *obj->getReceiveCb())->Call(
      isolate->GetCurrentContext()->Global(), 1, argv);
}

static void on_connection_callback(void *user_data, void *result) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  CloudWrapper *obj = reinterpret_cast<CloudWrapper *>(user_data);
  uint32_t ret = reinterpret_cast<uintptr_t>(result);

  log_dbg("");

  if (!obj || !obj->getConnectionCb())
    return;

  if (ret == ARTIK_WEBSOCKET_CLOSED) {
    Handle<Value> argv[] = {
      Handle<Value>(String::NewFromUtf8(isolate, "CLOSED")),
    };
    Local<Function>::New(isolate, *obj->getConnectionCb())->Call(
        isolate->GetCurrentContext()->Global(), 1, argv);
    return;
  } else if (ret == ARTIK_WEBSOCKET_CONNECTED) {
    Handle<Value> argv[] = {
      Handle<Value>(String::NewFromUtf8(isolate, "CONNECTED")),
    };
    Local<Function>::New(isolate, *obj->getConnectionCb())->Call(
        isolate->GetCurrentContext()->Global(), 1, argv);
    return;
  } else if (ret == ARTIK_WEBSOCKET_HANDSHAKE_ERROR) {
    Handle<Value> argv[] = {
      Handle<Value>(String::NewFromUtf8(isolate, "HANDSHAKE ERROR")),
    };
    Local<Function>::New(isolate, *obj->getConnectionCb())->Call(
        isolate->GetCurrentContext()->Global(), 1, argv);
    return;
  }

  log_err("Wrong value for callback result");
}

CloudWrapper::CloudWrapper(const char* token)
  : m_cloud(new Cloud(token)),
    m_loop(GlibLoop::Instance()) {
  m_loop->attach();
}

CloudWrapper::~CloudWrapper() {
  delete m_cloud;
  m_loop->detach();
}

void CloudWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "cloud"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "send_message", send_message);
  NODE_SET_PROTOTYPE_METHOD(tpl, "send_action", send_action);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_current_user_profile",
                            get_current_user_profile);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_user_devices", get_user_devices);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_user_device_types",
                            get_user_device_types);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_user_application_properties",
                            get_user_application_properties);
  NODE_SET_PROTOTYPE_METHOD(tpl, "add_device", add_device);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_device", get_device);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_device_token", get_device_token);
  NODE_SET_PROTOTYPE_METHOD(tpl, "update_device_token", update_device_token);
  NODE_SET_PROTOTYPE_METHOD(tpl, "delete_device_token", delete_device_token);
  NODE_SET_PROTOTYPE_METHOD(tpl, "delete_device", delete_device);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_device_properties",
                            get_device_properties);
  NODE_SET_PROTOTYPE_METHOD(tpl, "set_device_server_properties",
                            set_device_server_properties);
  NODE_SET_PROTOTYPE_METHOD(tpl, "sdr_start_registration",
                            sdr_start_registration);
  NODE_SET_PROTOTYPE_METHOD(tpl, "sdr_registration_status",
                            sdr_registration_status);
  NODE_SET_PROTOTYPE_METHOD(tpl, "sdr_complete_registration",
                            sdr_complete_registration);
  NODE_SET_PROTOTYPE_METHOD(tpl, "websocket_open_stream",
                            websocket_open_stream);
  NODE_SET_PROTOTYPE_METHOD(tpl, "websocket_send_message",
                            websocket_send_message);
  NODE_SET_PROTOTYPE_METHOD(tpl, "websocket_close_stream",
                            websocket_close_stream);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "cloud"),
               tpl->GetFunction());
}

void CloudWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  char* token = NULL;
  CloudWrapper* obj = NULL;

  if (args.IsConstructCall()) {
    if (args[0]->IsString()) {
        v8::String::Utf8Value param0(args[0]->ToString());
        token = *param0;
        obj = new CloudWrapper(token);
    } else {
      obj = new CloudWrapper(NULL);
    }

    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    const int argc = 1;
    Local<Value> argv[argc] = { args[0] };

    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, argc, argv).ToLocalChecked());
  }
}

template<typename Func>
static void async_call(const Func& func, const Local<Function> &callback) {
  CloudWork *work = new CloudWork(callback);

  artik_error ret = func(work);

  if (ret != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(ret));
    Handle<Value> argv[] = {
      Nan::New<String>(msg).ToLocalChecked(),
      Nan::Null()
    };

    Nan::Call(work->callback, 2, argv);
    delete work;
  }
}

void CloudWrapper::send_message(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CloudWrapper* obj = ObjectWrap::Unwrap<CloudWrapper>(args.Holder());
  Cloud* cloud = obj->getObj();
  std::unique_ptr<artik_ssl_config> ssl_config(nullptr);

  log_dbg("");

  if (!args[0]->IsString()  || !args[1]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[0]->ToString());
  char *device_id = *param0;
  v8::String::Utf8Value param1(args[1]->ToString());
  char *message = *param1;

  /* SSL Configuration */
  if (args[2]->IsObject()) {
    ssl_config = SSLConfigConverter::convert(isolate, args[2]);
    if (!ssl_config) {
      return;
    }
  }

  /* If callback is provided, make the call asynchronous */
  if (args[3]->IsFunction()) {
    auto send_message_cb = [&](CloudWork *work) {
        return cloud->send_message_async(device_id, message,
                                cloud_callback, work, ssl_config.get());
    };

    async_call(send_message_cb, args[3].As<Function>());
  } else { /* Otherwise make the call directly */
    char *response = NULL;
    artik_error ret = cloud->send_message(device_id, message, &response,
                ssl_config.get());

    if (ret != S_OK && !response) {
      std::string msg = "Error: " + std::string(error_msg(ret));
      isolate->ThrowException(Nan::New(msg).ToLocalChecked());
      return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

void CloudWrapper::send_action(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CloudWrapper* obj = ObjectWrap::Unwrap<CloudWrapper>(args.Holder());
  Cloud* cloud = obj->getObj();
  std::unique_ptr<artik_ssl_config> ssl_config(nullptr);

  log_dbg("");

  if (!args[0]->IsString()  || !args[1]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[0]->ToString());
  char *device_id = *param0;
  v8::String::Utf8Value param1(args[1]->ToString());
  char *action = *param1;

  /* SSL Configuration */
  if (args[2]->IsObject()) {
    ssl_config = SSLConfigConverter::convert(isolate, args[2]);
    if (!ssl_config) {
      return;
    }
  }

  /* If callback is provided, make the call asynchronous */
  if (args[3]->IsFunction()) {
    auto send_action_cb = [&](CloudWork *work) {
      return cloud->send_action_async(device_id, action,
                                      cloud_callback, work, ssl_config.get());
    };

    async_call(send_action_cb, args[3].As<Function>());
  } else { /* Otherwise make the call directly */
    char *response = NULL;
    artik_error ret = cloud->send_action(device_id, action, &response,
        ssl_config.get());

    if (ret != S_OK && !response) {
      std::string msg = "Error: " + std::string(error_msg(ret));
      isolate->ThrowException(Nan::New(msg).ToLocalChecked());
      return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

void CloudWrapper::get_current_user_profile(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CloudWrapper* obj = ObjectWrap::Unwrap<CloudWrapper>(args.Holder());
  Cloud* cloud = obj->getObj();
  std::unique_ptr<artik_ssl_config> ssl_config(nullptr);

  log_dbg("");

  /* SSL Configuration */
  if (args[0]->IsObject()) {
    ssl_config = SSLConfigConverter::convert(isolate, args[0]);
    if (!ssl_config) {
      return;
    }
  }

  /* If callback is provided, make the call asynchronous */
  if (args[1]->IsFunction()) {
    auto current_user_profile_cb = [&](CloudWork* work) {
      return cloud->get_current_user_profile_async(
          cloud_callback, work, ssl_config.get());
    };

    async_call(current_user_profile_cb, args[1].As<Function>());
  } else { /* Otherwise make the call directly */
    char *response = NULL;
    artik_error ret =
      cloud->get_current_user_profile(&response, ssl_config.get());

    if (ret != S_OK && !response) {
      std::string msg = "Error: " + std::string(error_msg(ret));
      isolate->ThrowException(Nan::New(msg).ToLocalChecked());
      return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

void CloudWrapper::get_user_devices(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CloudWrapper* obj = ObjectWrap::Unwrap<CloudWrapper>(args.Holder());
  Cloud* cloud = obj->getObj();
  std::unique_ptr<artik_ssl_config> ssl_config(nullptr);

  log_dbg("");

  if (!args[0]->IsNumber() || !args[1]->IsBoolean()
      || !args[2]->IsNumber() || !args[3]->IsString() ) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  int count = args[0]->IntegerValue();
  bool properties = args[1]->BooleanValue();
  int offset = args[2]->IntegerValue();
  v8::String::Utf8Value param3(args[3]->ToString());
  char *user_id = *param3;

  /* SSL Configuration */
  if (args[4]->IsObject()) {
    ssl_config = SSLConfigConverter::convert(isolate, args[4]);
    if (!ssl_config) {
      return;
    }
  }

  /* If callback is provided, make the call asynchronous */
  if (args[5]->IsFunction()) {
    auto get_user_devices_cb = [&](CloudWork *work) {
      return cloud->get_user_devices_async(count, properties, offset,
                            user_id, cloud_callback, work, ssl_config.get());
    };

    async_call(get_user_devices_cb, args[5].As<Function>());
  } else { /* Otherwise make the call directly */
    char *response = NULL;
    artik_error ret = cloud->get_user_devices(count, properties, offset,
        user_id, &response, ssl_config.get());

    if (ret != S_OK && !response) {
      std::string msg = "Error: " + std::string(error_msg(ret));
      isolate->ThrowException(Nan::New(msg).ToLocalChecked());
      return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

void CloudWrapper::get_user_device_types(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CloudWrapper* obj = ObjectWrap::Unwrap<CloudWrapper>(args.Holder());
  Cloud* cloud = obj->getObj();
  std::unique_ptr<artik_ssl_config> ssl_config(nullptr);

  log_dbg("");

  if (!args[0]->IsNumber() || !args[1]->IsBoolean()
      || !args[2]->IsNumber() || !args[3]->IsString()  ) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  int count = args[0]->IntegerValue();
  bool shared = args[1]->BooleanValue();
  int offset = args[2]->IntegerValue();
  v8::String::Utf8Value param3(args[3]->ToString());
  char *user_id = *param3;

  /* SSL Configuration */
  if (args[4]->IsObject()) {
    ssl_config = SSLConfigConverter::convert(isolate, args[4]);
    if (!ssl_config) {
      return;
    }
  }

  /* If callback is provided, make the call asynchronous */
  if (args[5]->IsFunction()) {
    auto get_user_device_types_cb = [&](CloudWork* work) {
      return cloud->get_user_device_types_async(count, shared, offset, user_id,
                                cloud_callback, work, ssl_config.get());
    };

    async_call(get_user_device_types_cb, args[5].As<Function>());
  } else { /* Otherwise make the call directly */
    char *response = NULL;
    artik_error ret = cloud->get_user_device_types(count, shared, offset,
        user_id, &response, ssl_config.get());

    if (ret != S_OK && !response) {
      std::string msg = "Error: " + std::string(error_msg(ret));
      isolate->ThrowException(Nan::New(msg).ToLocalChecked());
      return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

void CloudWrapper::get_user_application_properties(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CloudWrapper* obj = ObjectWrap::Unwrap<CloudWrapper>(args.Holder());
  Cloud* cloud = obj->getObj();
  std::unique_ptr<artik_ssl_config> ssl_config(nullptr);

  log_dbg("");

  if (!args[0]->IsString() || !args[1]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[0]->ToString());
  v8::String::Utf8Value param1(args[1]->ToString());
  const char *device_id = *param0;
  const char *app_id = *param1;

  /* SSL Configuration */
  if (args[2]->IsObject()) {
    ssl_config = SSLConfigConverter::convert(isolate, args[2]);
    if (!ssl_config) {
      return;
    }
  }

  /* If callback is provided, make the call asynchronous */
  if (args[3]->IsFunction()) {
    auto get_user_application_properties_cb = [&](CloudWork *work) {
          return cloud->get_user_application_properties_async(device_id, app_id,
                                    cloud_callback, work, ssl_config.get());
    };

    async_call(get_user_application_properties_cb, args[3].As<Function>());
  } else { /* Otherwise make the call directly */
    char *response = NULL;
    artik_error ret = cloud->get_user_application_properties(device_id, app_id,
        &response, ssl_config.get());

    if (ret != S_OK && !response) {
      std::string msg = "Error: " + std::string(error_msg(ret));
      isolate->ThrowException(Nan::New(msg).ToLocalChecked());
      return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

void CloudWrapper::get_device(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CloudWrapper* obj = ObjectWrap::Unwrap<CloudWrapper>(args.Holder());
  Cloud* cloud = obj->getObj();
  bool properties = false;
  std::unique_ptr<artik_ssl_config> ssl_config(nullptr);

  log_dbg("");

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[0]->ToString());
  const char *device_id = *param0;

  if (args[1]->IsBoolean())
    properties = args[1]->BooleanValue();

  /* SSL Configuration */
  if (args[2]->IsObject()) {
    ssl_config = SSLConfigConverter::convert(isolate, args[2]);
    if (!ssl_config) {
      return;
    }
  }

  /* If callback is provided, make the call asynchronous */
  if (args[3]->IsFunction()) {
    auto get_device_cb = [&](CloudWork* work) {
      return cloud->get_device_async(device_id, properties,
                                     cloud_callback, work, ssl_config.get());
    };

    async_call(get_device_cb, args[3].As<Function>());
  } else { /* Otherwise make the call directly */
    char *response = NULL;
    artik_error ret = cloud->get_device(device_id, properties, &response,
        ssl_config.get());

    if (ret != S_OK && !response) {
      std::string msg = "Error: " + std::string(error_msg(ret));
      isolate->ThrowException(Nan::New(msg).ToLocalChecked());
      return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

void CloudWrapper::get_device_token(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CloudWrapper* obj = ObjectWrap::Unwrap<CloudWrapper>(args.Holder());
  Cloud* cloud = obj->getObj();
  std::unique_ptr<artik_ssl_config> ssl_config(nullptr);

  log_dbg("");

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[0]->ToString());
  const char *device_id = *param0;

  /* SSL Configuration */
  if (args[1]->IsObject()) {
    ssl_config = SSLConfigConverter::convert(isolate, args[1]);
    if (!ssl_config) {
      return;
    }
  }

  /* If callback is provided, make the call asynchronous */
  if (args[2]->IsFunction()) {
    auto get_device_token_cb = [&](CloudWork *work) {
      return cloud->get_device_token_async(device_id,
                                cloud_callback, work, ssl_config.get());
    };

    async_call(get_device_token_cb, args[2].As<Function>());
  } else { /* Otherwise make the call directly */
    char *response = NULL;
    artik_error ret =
      cloud->get_device_token(device_id, &response, ssl_config.get());

    if (ret != S_OK && !response) {
      std::string msg = "Error: " + std::string(error_msg(ret));
      isolate->ThrowException(Nan::New(msg).ToLocalChecked());
      return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

void CloudWrapper::add_device(const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CloudWrapper* obj = ObjectWrap::Unwrap<CloudWrapper>(args.Holder());
  Cloud* cloud = obj->getObj();
  std::unique_ptr<artik_ssl_config> ssl_config(nullptr);

  log_dbg("");

  if (!args[0]->IsString() || !args[1]->IsString() || !args[2]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[0]->ToString());
  v8::String::Utf8Value param1(args[1]->ToString());
  v8::String::Utf8Value param2(args[2]->ToString());
  const char *user_id = *param0;
  const char *dt_id = *param1;
  const char *name = *param2;

  /* SSL Configuration */
  if (args[3]->IsObject()) {
    ssl_config = SSLConfigConverter::convert(isolate, args[3]);
    if (!ssl_config) {
      return;
    }
  }

  /* If callback is provided, make the call asynchronous */
  if (args[4]->IsFunction()) {
    auto add_device_cb = [&](CloudWork *work) {
      return cloud->add_device_async(user_id, dt_id, name,
                                     cloud_callback, work, ssl_config.get());
    };

    async_call(add_device_cb, args[4].As<Function>());
  } else { /* Otherwise make the call directly */
    char *response = NULL;
    artik_error ret = cloud->add_device(user_id, dt_id, name, &response,
        ssl_config.get());

    if (ret != S_OK && !response) {
      std::string msg = "Error: " + std::string(error_msg(ret));
      isolate->ThrowException(Nan::New(msg).ToLocalChecked());
      return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

void CloudWrapper::update_device_token(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CloudWrapper* obj = ObjectWrap::Unwrap<CloudWrapper>(args.Holder());
  Cloud* cloud = obj->getObj();
  std::unique_ptr<artik_ssl_config> ssl_config(nullptr);

  log_dbg("");

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[0]->ToString());
  const char *device_id = *param0;

  /* SSL Configuration */
  if (args[1]->IsObject()) {
    ssl_config = SSLConfigConverter::convert(isolate, args[1]);
    if (!ssl_config) {
      return;
    }
  }

  /* If callback is provided, make the call asynchronous */
  if (args[2]->IsFunction()) {
    auto update_device_token_cb = [&](CloudWork *work) {
      return cloud->update_device_token_async(device_id,
                                    cloud_callback, work, ssl_config.get());
    };

    async_call(update_device_token_cb, args[2].As<Function>());
  } else { /* Otherwise make the call directly */
    char *response = NULL;
    artik_error ret = cloud->update_device_token(device_id, &response,
        ssl_config.get());

    if (ret != S_OK && !response) {
      std::string msg = "Error: " + std::string(error_msg(ret));
      isolate->ThrowException(Nan::New(msg).ToLocalChecked());
      return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

void CloudWrapper::delete_device_token(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CloudWrapper* obj = ObjectWrap::Unwrap<CloudWrapper>(args.Holder());
  Cloud* cloud = obj->getObj();
  std::unique_ptr<artik_ssl_config> ssl_config(nullptr);

  log_dbg("");

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[0]->ToString());
  const char *device_id = *param0;

  /* SSL Configuration */
  if (args[1]->IsObject()) {
    ssl_config = SSLConfigConverter::convert(isolate, args[1]);
    if (!ssl_config) {
      return;
    }
  }

  /* If callback is provided, make the call asynchronous */
  if (args[2]->IsFunction()) {
    auto delete_device_token_cb = [&](CloudWork* work) {
      return cloud->delete_device_token_async(device_id,
                                    cloud_callback, work, ssl_config.get());
    };

    async_call(delete_device_token_cb, args[2].As<Function>());
  } else { /* Otherwise make the call directly */
    char *response = NULL;
    artik_error ret = cloud->delete_device_token(device_id, &response,
        ssl_config.get());

    if (ret != S_OK && !response) {
      std::string msg = "Error: " + std::string(error_msg(ret));
      isolate->ThrowException(Nan::New(msg).ToLocalChecked());
      return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

void CloudWrapper::delete_device(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CloudWrapper* obj = ObjectWrap::Unwrap<CloudWrapper>(args.Holder());
  Cloud* cloud = obj->getObj();
  std::unique_ptr<artik_ssl_config> ssl_config(nullptr);

  log_dbg("");

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[0]->ToString());
  const char *device_id = *param0;

  /* SSL Configuration */
  if (args[1]->IsObject()) {
    ssl_config = SSLConfigConverter::convert(isolate, args[1]);
    if (!ssl_config) {
      return;
    }
  }

  /* If callback is provided, make the call asynchronous */
  if (args[2]->IsFunction()) {
    auto delete_device_cb = [&](CloudWork* work) {
      return cloud->delete_device_async(device_id,
                                    cloud_callback, work, ssl_config.get());
    };

    async_call(delete_device_cb, args[2].As<Function>());
  } else { /* Otherwise make the call directly */
    char *response = NULL;
    artik_error ret =
      cloud->delete_device(device_id, &response, ssl_config.get());

    if (ret != S_OK && !response) {
      std::string msg = "Error: " + std::string(error_msg(ret));
      isolate->ThrowException(Nan::New(msg).ToLocalChecked());
      return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

void CloudWrapper::get_device_properties(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CloudWrapper* obj = ObjectWrap::Unwrap<CloudWrapper>(args.Holder());
  Cloud *cloud = obj->getObj();
  std::unique_ptr<artik_ssl_config> ssl_config(nullptr);

  log_dbg("");

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[0]->ToString());
  const char *device_id = *param0;
  if (!args[1]->IsBoolean()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  bool timestamp = args[1]->BooleanValue();

  /* SSL Configuration */
  if (args[2]->IsObject()) {
    ssl_config = SSLConfigConverter::convert(isolate, args[2]);
    if (!ssl_config) {
      return;
    }
  }

  /* If callback is provided, make the call asynchronous */
  if (args[3]->IsFunction()) {
    auto get_device_properties_cb = [&](CloudWork *work) {
      return cloud->get_device_properties_async(device_id, timestamp,
                                cloud_callback, work, ssl_config.get());
      };

    async_call(get_device_properties_cb, args[3].As<Function>());
  } else { /* Otherwise make the call directly */
    char *response = NULL;
    artik_error ret = cloud->get_device_properties(device_id, timestamp,
                                                  &response, ssl_config.get());

    if (ret != S_OK && !response) {
      std::string msg = "Error: " + std::string(error_msg(ret));
      isolate->ThrowException(Nan::New(msg).ToLocalChecked());
      return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

void CloudWrapper::set_device_server_properties(
      const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CloudWrapper* obj = ObjectWrap::Unwrap<CloudWrapper>(args.Holder());
  Cloud *cloud = obj->getObj();
  std::unique_ptr<artik_ssl_config> ssl_config(nullptr);

  log_dbg("");

  if (!args[0]->IsString() || !args[1]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[0]->ToString());
  v8::String::Utf8Value param1(args[1]->ToString());
  const char *device_id = *param0;
  const char *data = *param1;

  /* SSL Configuration */
  if (args[2]->IsObject()) {
    ssl_config = SSLConfigConverter::convert(isolate, args[2]);
    if (!ssl_config) {
      return;
    }
  }

  /* If callback is provided, make the call asynchronous */
  if (args[3]->IsFunction()) {
    auto set_device_server_properties_cb = [&](CloudWork *work) {
      return cloud->set_device_server_properties_async(device_id, data,
                                    cloud_callback, work, ssl_config.get());
    };

    async_call(set_device_server_properties_cb, args[3].As<Function>());
  } else { /* Otherwise make the call directly */
    char *response = NULL;
    artik_error ret = cloud->set_device_server_properties(
      device_id, data,
      &response, ssl_config.get());

    if (ret != S_OK && !response) {
      std::string msg = "Error: " + std::string(error_msg(ret));
      isolate->ThrowException(Nan::New(msg).ToLocalChecked());
      return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

void CloudWrapper::sdr_start_registration(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CloudWrapper* obj = ObjectWrap::Unwrap<CloudWrapper>(args.Holder());
  Cloud* cloud = obj->getObj();

  log_dbg("");

  if (!args[0]->IsString() || !args[1]->IsString() || !args[2]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[0]->ToString());
  v8::String::Utf8Value param1(args[1]->ToString());
  v8::String::Utf8Value param2(args[2]->ToString());

  auto cert_id =
    to_artik_parameter<artik_security_certificate_id>(
      SSLConfigConverter::security_certificate_ids, *param0);
  if (!cert_id) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,
      "Wrong value of cert_id. ")));
    return;
  }
  const char *dt_id = *param1;
  const char *vendor_id = *param2;

  /* If callback is provided, make the call asynchronous */
  if (args[3]->IsFunction()) {
    auto sdr_start_registration_cb = [&](CloudWork *work) {
      return cloud->sdr_start_registration_async(cert_id.value(),
                            dt_id, vendor_id,  cloud_callback, work);
    };

    async_call(sdr_start_registration_cb, args[3].As<Function>());
  } else { /* Otherwise make the call directly */
    char *response = NULL;
    artik_error ret = cloud->sdr_start_registration(cert_id.value(),
        dt_id, vendor_id, &response);

    if (ret != S_OK && !response) {
      std::string msg = "Error: " + std::string(error_msg(ret));
      isolate->ThrowException(Nan::New(msg).ToLocalChecked());
      return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

void CloudWrapper::sdr_registration_status(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CloudWrapper* obj = ObjectWrap::Unwrap<CloudWrapper>(args.Holder());
  Cloud* cloud = obj->getObj();

  log_dbg("");

  if (!args[0]->IsString() || !args[1]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[0]->ToString());
  v8::String::Utf8Value param1(args[1]->ToString());

  const char *reg_id = *param1;
  auto cert_id =
    to_artik_parameter<artik_security_certificate_id>(
      SSLConfigConverter::security_certificate_ids, *param0);
  if (!cert_id) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,
      "Wrong value of cert_id. ")));
    return;
  }

  /* If callback is provided, make the call asynchronous */
  if (args[2]->IsFunction()) {
    auto sdr_registration_status_cb = [&](CloudWork *work) {
      return cloud->sdr_registration_status_async(cert_id.value(), reg_id,
                                cloud_callback, work);
    };

    async_call(sdr_registration_status_cb, args[2].As<Function>());
  } else { /* Otherwise make the call directly */
    char *response = NULL;
    artik_error ret = cloud->sdr_registration_status(cert_id.value(), reg_id,
        &response);

    if (ret != S_OK && !response) {
      std::string msg = "Error: " + std::string(error_msg(ret));
      isolate->ThrowException(Nan::New(msg).ToLocalChecked());
      return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

void CloudWrapper::sdr_complete_registration(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CloudWrapper* obj = ObjectWrap::Unwrap<CloudWrapper>(args.Holder());
  Cloud* cloud = obj->getObj();

  log_dbg("");

  if (!args[0]->IsString() || !args[1]->IsString() || !args[2]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[0]->ToString());
  v8::String::Utf8Value param1(args[1]->ToString());
  v8::String::Utf8Value param2(args[2]->ToString());

    const char *reg_id = *param1;
    const char *nonce = *param2;
  auto cert_id =
    to_artik_parameter<artik_security_certificate_id>(
      SSLConfigConverter::security_certificate_ids, *param0);
  if (!cert_id) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(isolate,
      "Wrong value of cert_id. ")));
    return;
  }

  /* If callback is provided, make the call asynchronous */
  if (args[3]->IsFunction()) {
    auto sdr_complete_registration_cb = [&](CloudWork *work) {
      return cloud->sdr_complete_registration_async(cert_id.value(), reg_id,
                                                nonce, cloud_callback, work);
    };

    async_call(sdr_complete_registration_cb, args[3].As<Function>());
  } else { /* Otherwise make the call directly */
    char *response = NULL;
    artik_error ret = cloud->sdr_complete_registration(cert_id.value(), reg_id,
        nonce, &response);

    if (ret != S_OK && !response) {
      std::string msg = "Error: " + std::string(error_msg(ret));
      isolate->ThrowException(Nan::New(msg).ToLocalChecked());
      return;
    }

    args.GetReturnValue().Set(String::NewFromUtf8(isolate, response));

    free(response);
  }
}

void CloudWrapper::websocket_open_stream(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CloudWrapper* obj = ObjectWrap::Unwrap<CloudWrapper>(args.Holder());
  Cloud* cloud = obj->getObj();
  artik_error ret = S_OK;
  std::unique_ptr<artik_ssl_config> ssl_config(nullptr);

  log_dbg("");

  if (!args[0]->IsString() || !args[1]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[0]->ToString());
  v8::String::Utf8Value param1(args[1]->ToString());

  char *token = *param0;
  char *device_id = *param1;

  /* SSL Configuration */
  if (args[2]->IsObject()) {
    ssl_config = SSLConfigConverter::convert(isolate, args[2]);
    if (!ssl_config) {
      return;
    }
  }

  ret = cloud->websocket_open_stream(token, device_id, ssl_config.get());

  /* If a callback is provided, use it for notification */
  if (ret == S_OK && args[3]->IsFunction()) {
    obj->m_receive_cb = new v8::Persistent<v8::Function>();
    obj->m_receive_cb->Reset(isolate, Local<Function>::Cast(args[3]));
    cloud->websocket_set_receive_callback(on_receive_callback,
        reinterpret_cast<void*>(obj));
  }

  if (ret == S_OK && args[4]->IsFunction()) {
    obj->m_connection_cb = new v8::Persistent<v8::Function>();
    obj->m_connection_cb->Reset(isolate, Local<Function>::Cast(args[4]));
    cloud->websocket_set_connection_callback(on_connection_callback,
        reinterpret_cast<void*>(obj));
  }

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, error_msg(ret)));
}

void CloudWrapper::websocket_send_message(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CloudWrapper* obj = ObjectWrap::Unwrap<CloudWrapper>(args.Holder());
  Cloud* cloud = obj->getObj();
  artik_error ret = S_OK;

  log_dbg("");

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[0]->ToString());
  char *msg = *param0;
  ret = cloud->websocket_send_message(msg);

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, error_msg(ret)));
}

void CloudWrapper::websocket_close_stream(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  CloudWrapper* obj = ObjectWrap::Unwrap<CloudWrapper>(args.Holder());
  Cloud* cloud = obj->getObj();
  artik_error ret = S_OK;

  log_dbg("");

  ret = cloud->websocket_close_stream();
  if (ret == S_OK) {
    delete(obj->m_receive_cb);
    obj->m_receive_cb = NULL;
    delete(obj->m_connection_cb);
    obj->m_connection_cb = NULL;
  }

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, error_msg(ret)));
}

}  // namespace artik
