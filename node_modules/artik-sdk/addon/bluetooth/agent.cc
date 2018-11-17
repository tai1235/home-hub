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

#include "bluetooth/agent.h"

#include <artik_log.h>
#include <glib.h>
#include <utils.h>

#include <string>

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
using v8::HandleScope;
using v8::Int32;
using v8::Uint32;
using v8::Boolean;
using v8::Context;

Persistent<Function> AgentWrapper::constructor;
std::array<const char*, 5> AgentWrapper::capabilities = {
  "keyboard_display",
  "display_only",
  "display_yesno",
  "keyboard_only",
  "no_input_no_output"
};

Persistent<Function> AgentRequestWrapper::constructor;
std::array<const char*, 2> AgentRequestWrapper::error_types = {
  "rejected",
  "canceled"
};

static void agent_release(artik_bt_event event,
    void *data, void *user_data) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope handleScope(isolate);
  AgentWrapper* wrap = reinterpret_cast<AgentWrapper*>(user_data);

  Local<Function>::New(isolate, *wrap->getReleaseCb())->Call(
      isolate->GetCurrentContext()->Global(), 0, NULL);
}

static void agent_request_pincode(artik_bt_event event,
    void *data, void *user_data) {
  artik_bt_agent_request_property *request_property =
    reinterpret_cast<artik_bt_agent_request_property *>(data);

  Isolate *isolate = Isolate::GetCurrent();
  HandleScope handleScope(isolate);
  AgentWrapper* wrap = reinterpret_cast<AgentWrapper*>(user_data);

  Local<Value> agentRequest = AgentRequestWrapper::newInstance(
      request_property->handle);
  Handle<Value> argv[] = {
    agentRequest,
    String::NewFromUtf8(isolate, request_property->device)
  };

  Local<Function>::New(isolate, *wrap->getRequestPincodeCb())->Call(
      isolate->GetCurrentContext()->Global(), 2, argv);
}

static void agent_display_pincode(artik_bt_event event,
    void *data, void *user_data) {
  artik_bt_agent_pincode_property *pincode_property =
    reinterpret_cast<artik_bt_agent_pincode_property *>(data);

  Isolate *isolate = Isolate::GetCurrent();
  HandleScope handleScope(isolate);
  AgentWrapper* wrap = reinterpret_cast<AgentWrapper*>(user_data);

  Handle<Value> argv[] = {
    String::NewFromUtf8(isolate, pincode_property->device),
    String::NewFromUtf8(isolate, pincode_property->pincode)
  };
  Local<Function>::New(isolate, *wrap->getDisplayPincodeCb())->Call(
      isolate->GetCurrentContext()->Global(), 2, argv);
}

static void agent_request_passkey(artik_bt_event event,
    void *data, void *user_data) {
  artik_bt_agent_request_property *request_property =
    reinterpret_cast<artik_bt_agent_request_property *>(data);

  Isolate *isolate = Isolate::GetCurrent();
  HandleScope handleScope(isolate);
  AgentWrapper* wrap = reinterpret_cast<AgentWrapper*>(user_data);

  Local<Object> agentRequest = AgentRequestWrapper::newInstance(
      request_property->handle);

  Local<Value> argv[] = {
    agentRequest,
    String::NewFromUtf8(isolate, request_property->device)
  };

  Local<Function>::New(isolate, *wrap->getRequestPasskeyCb())->Call(
      isolate->GetCurrentContext()->Global(), 2, argv);
}

static void agent_display_passkey(artik_bt_event event,
    void *data, void *user_data) {
  artik_bt_agent_passkey_property *passkey_property =
    reinterpret_cast<artik_bt_agent_passkey_property *>(data);

  Isolate *isolate = Isolate::GetCurrent();
  HandleScope handleScope(isolate);
  AgentWrapper* wrap = reinterpret_cast<AgentWrapper*>(user_data);

  Handle<Value> argv[] = {
    String::NewFromUtf8(isolate, passkey_property->device),
    Uint32::New(isolate, passkey_property->passkey),
    Uint32::New(isolate, passkey_property->entered)
  };
  Local<Function>::New(isolate, *wrap->getDisplayPasskeyCb())->Call(
      isolate->GetCurrentContext()->Global(), 3, argv);
}

static void agent_request_confirmation(artik_bt_event event,
    void *data, void *user_data) {
  artik_bt_agent_confirmation_property *confirmation_property =
    reinterpret_cast<artik_bt_agent_confirmation_property *>(data);

  Isolate *isolate = Isolate::GetCurrent();
  HandleScope handleScope(isolate);
  AgentWrapper *wrap = reinterpret_cast<AgentWrapper*>(user_data);

  Local<Value> agentRequest = AgentRequestWrapper::newInstance(
      confirmation_property->handle);
  Handle<Value> argv[] = {
    agentRequest,
    String::NewFromUtf8(isolate, confirmation_property->device),
    Uint32::New(isolate, confirmation_property->passkey)
  };
  Local<Function>::New(isolate, *wrap->getRequestConfirmationCb())->Call(
      isolate->GetCurrentContext()->Global(), 3, argv);
}

static void agent_request_authorization(artik_bt_event event,
    void *data, void *user_data) {
  artik_bt_agent_request_property *request_property =
    reinterpret_cast<artik_bt_agent_request_property *>(data);

  Isolate *isolate = Isolate::GetCurrent();
  HandleScope handleScope(isolate);
  AgentWrapper *wrap = reinterpret_cast<AgentWrapper*>(user_data);

  Local<Value> agentRequest = AgentRequestWrapper::newInstance(
      request_property->handle);
  Handle<Value> argv[] = {
    agentRequest,
    String::NewFromUtf8(isolate, request_property->device)
  };
  Local<Function>::New(isolate, *wrap->getRequestAuthorizationCb())->Call(
      isolate->GetCurrentContext()->Global(), 2, argv);
}

static void agent_authorize_service(artik_bt_event event,
    void *data, void *user_data) {
  artik_bt_agent_authorize_property *authorize_property =
    reinterpret_cast<artik_bt_agent_authorize_property *>(data);

  Isolate *isolate = Isolate::GetCurrent();
  HandleScope handleScope(isolate);
  AgentWrapper *wrap = reinterpret_cast<AgentWrapper*>(user_data);

  Local<Value> agentRequest = AgentRequestWrapper::newInstance(
      authorize_property->handle);
  Handle<Value> argv[] = {
    agentRequest,
    String::NewFromUtf8(isolate, authorize_property->device),
    String::NewFromUtf8(isolate, authorize_property->uuid)
  };
  Local<Function>::New(isolate, *wrap->getAuthorizeServiceCb())->Call(
      isolate->GetCurrentContext()->Global(), 3, argv);
}

static void agent_cancel(artik_bt_event event,
    void *data, void *user_data) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope handleScope(isolate);
  AgentWrapper *wrap = reinterpret_cast<AgentWrapper*>(user_data);

  Local<Function>::New(isolate, *wrap->getCancelCb())->Call(
      isolate->GetCurrentContext()->Global(), 0, NULL);
}

AgentRequestWrapper::AgentRequestWrapper()
  : m_bt(new Bluetooth()),  m_handle(nullptr) {
}

AgentRequestWrapper::~AgentRequestWrapper() {
  delete m_bt;
}

Local<Object> AgentRequestWrapper::newInstance(
    artik_bt_agent_request_handle handle) {
  Isolate *isolate = Isolate::GetCurrent();

  Local<Context> context = isolate->GetCurrentContext();
  Local<Function> cons = Local<Function>::New(isolate, constructor);
  Local<Object> obj = cons->NewInstance(context, 0, NULL).ToLocalChecked();

  AgentRequestWrapper *wrap = ObjectWrap::Unwrap<AgentRequestWrapper>(obj);
  wrap->setRequestHandle(handle);

  return obj;
}

void AgentRequestWrapper::Init(v8::Local<v8::Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "AgentRequest"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "send_pincode", agent_send_pincode);
  NODE_SET_PROTOTYPE_METHOD(tpl, "send_passkey", agent_send_passkey);
  NODE_SET_PROTOTYPE_METHOD(tpl, "send_error", agent_send_error);
  NODE_SET_PROTOTYPE_METHOD(tpl, "send_empty_response",
      agent_send_empty_response);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "AgentRequest"),
      tpl->GetFunction());
}

void AgentRequestWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  log_dbg("");

  if (args.IsConstructCall()) {
    if (args.Length() != 0) {
      isolate->ThrowException(Exception::TypeError(
              String::NewFromUtf8(isolate, "Wrong number of arguments")));
      return;
    }

    AgentRequestWrapper* obj = new AgentRequestWrapper();
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

void AgentRequestWrapper::agent_send_pincode(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  AgentRequestWrapper* obj = ObjectWrap::Unwrap<AgentRequestWrapper>(
      args.Holder());
  Bluetooth *bt = obj->getObj();

  log_dbg("");

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments type")));
    return;
  }

  String::Utf8Value val(args[0]->ToString());
  artik_error err = bt->agent_send_pincode(obj->m_handle, *val);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void AgentRequestWrapper::agent_send_passkey(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  AgentRequestWrapper* obj = ObjectWrap::Unwrap<AgentRequestWrapper>(
      args.Holder());
  Bluetooth *bt = obj->getObj();

  log_dbg("");

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  v8::Maybe<int64_t> val = args[0]->IntegerValue(v8::Context::New(isolate));
  if (val.IsNothing()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments type")));
    return;
  }

  artik_error err = bt->agent_send_passkey(obj->m_handle, val.FromJust());
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void AgentRequestWrapper::agent_send_error(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  AgentRequestWrapper* obj = ObjectWrap::Unwrap<AgentRequestWrapper>(
      args.Holder());
  Bluetooth *bt = obj->getObj();

  log_dbg("");

  if (args.Length() != 2) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsString() || !args[1]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments type")));
    return;
  }

  String::Utf8Value error_type(args[0]->ToString());
  auto type = to_artik_parameter<artik_bt_agent_request_error>(error_types,
      *error_type);
  if (!type) {
    std::string err = "Error type " + std::string(*error_type) +
        " is not supported";
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, err.c_str())));
    return;
  }

  String::Utf8Value err_msg(args[1]->ToString());
  artik_error err = bt->agent_send_error(obj->m_handle, type.value(), *err_msg);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void AgentRequestWrapper::agent_send_empty_response(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  AgentRequestWrapper* obj = ObjectWrap::Unwrap<AgentRequestWrapper>(
      args.Holder());
  Bluetooth *bt = obj->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  artik_error err = bt->agent_send_empty_response(obj->m_handle);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void AgentRequestWrapper::setRequestHandle(
    artik_bt_agent_request_handle handle) {
  m_handle = handle;
}

AgentWrapper::AgentWrapper(
    v8::Persistent<v8::Function> *release_cb,
    v8::Persistent<v8::Function> *request_pincode_cb,
    v8::Persistent<v8::Function> *display_pincode_cb,
    v8::Persistent<v8::Function> *request_passkey_cb,
    v8::Persistent<v8::Function> *display_passkey_cb,
    v8::Persistent<v8::Function> *request_confirmation_cb,
    v8::Persistent<v8::Function> *request_authorization_cb,
    v8::Persistent<v8::Function> *authorize_service_cb,
    v8::Persistent<v8::Function> *cancel_cb)
  : m_bt(new Bluetooth()),
    m_release_cb(release_cb),
    m_request_pincode_cb(request_pincode_cb),
    m_display_pincode_cb(display_pincode_cb),
    m_request_passkey_cb(request_passkey_cb),
    m_display_passkey_cb(display_passkey_cb),
    m_request_confirmation_cb(request_confirmation_cb),
    m_request_authorization_cb(request_authorization_cb),
    m_authorize_service_cb(authorize_service_cb),
    m_cancel_cb(cancel_cb) {
  m_loop = GlibLoop::Instance();
  m_loop->attach();
}

AgentWrapper::~AgentWrapper() {
  delete m_bt;
  delete m_release_cb;
  delete m_request_pincode_cb;
  delete m_display_pincode_cb;
  delete m_request_passkey_cb;
  delete m_display_passkey_cb;
  delete m_request_confirmation_cb;
  delete m_request_authorization_cb;
  delete m_cancel_cb;
  m_loop->detach();
}

void AgentWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "Agent"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "register_capability",
      agent_register_capability);
  NODE_SET_PROTOTYPE_METHOD(tpl, "set_default", agent_set_default);
  NODE_SET_PROTOTYPE_METHOD(tpl, "unregister", agent_unregister);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "Agent"),
         tpl->GetFunction());
}

void AgentWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  log_dbg("");
  if (args.Length() != 9) {
    isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, "Wrong number of arguments")));
      return;
  }

  if (args.IsConstructCall()) {
    AgentWrapper* obj = new AgentWrapper(
        new Persistent<Function>(isolate, Local<Function>::Cast(args[0])),
        new Persistent<Function>(isolate, Local<Function>::Cast(args[1])),
        new Persistent<Function>(isolate, Local<Function>::Cast(args[2])),
        new Persistent<Function>(isolate, Local<Function>::Cast(args[3])),
        new Persistent<Function>(isolate, Local<Function>::Cast(args[4])),
        new Persistent<Function>(isolate, Local<Function>::Cast(args[5])),
        new Persistent<Function>(isolate, Local<Function>::Cast(args[6])),
        new Persistent<Function>(isolate, Local<Function>::Cast(args[7])),
        new Persistent<Function>(isolate, Local<Function>::Cast(args[8])));
    obj->Wrap(args.This());

    artik_bt_callback_property agent_callbacks[] = {
        {BT_EVENT_AGENT_RELEASE, agent_release, reinterpret_cast<void *>(obj)},
        {BT_EVENT_AGENT_REQUEST_PINCODE, agent_request_pincode,
            reinterpret_cast<void *>(obj)},
        {BT_EVENT_AGENT_DISPLAY_PINCODE, agent_display_pincode,
            reinterpret_cast<void *>(obj)},
        {BT_EVENT_AGENT_REQUEST_PASSKEY, agent_request_passkey,
            reinterpret_cast<void *>(obj)},
        {BT_EVENT_AGENT_DISPLAY_PASSKEY, agent_display_passkey,
            reinterpret_cast<void *>(obj)},
        {BT_EVENT_AGENT_CONFIRM, agent_request_confirmation,
            reinterpret_cast<void *>(obj)},
        {BT_EVENT_AGENT_AUTHORIZE, agent_request_authorization,
            reinterpret_cast<void *>(obj)},
        {BT_EVENT_AGENT_AUTHORIZE_SERVICE, agent_authorize_service,
            reinterpret_cast<void *>(obj)},
        {BT_EVENT_AGENT_CANCEL, agent_cancel,
            reinterpret_cast<void *>(obj)}
    };

    obj->getObj()->set_callbacks(agent_callbacks, 9);

    args.GetReturnValue().Set(args.This());
  } else {
    int argc = 9;
    Local<Value> argv[argc];

    for (int i = 0; i < argc; ++i)
      argv[i] = args[i];

    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, argc, argv).ToLocalChecked());
  }
}

void AgentWrapper::agent_register_capability(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<AgentWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (!args[0]->IsString()) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments type")));
    return;
  }

  String::Utf8Value val(args[0]->ToString());
  auto capability = to_artik_parameter<artik_bt_agent_capability>(
      capabilities, *val);
  if (!capability) {
    std::string err = "Capability type " + std::string(*val) +
        " is not supported";
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, err.c_str())));
    return;
  }
  artik_error err = bt->agent_register_capability(capability.value());
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void AgentWrapper::agent_set_default(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<AgentWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  artik_error err = bt->agent_set_default();
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void AgentWrapper::agent_unregister(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<AgentWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  artik_error err = bt->agent_unregister();

  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

}  // namespace artik

