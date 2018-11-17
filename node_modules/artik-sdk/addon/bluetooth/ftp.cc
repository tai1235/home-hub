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

#include "bluetooth/ftp.h"

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
using v8::Integer;
using v8::Boolean;
using v8::Context;

Persistent<Function> FtpWrapper::constructor;

static void _ftp_event_callback(artik_bt_event event, void *data,
    void *user_data) {
  Isolate *isolate = Isolate::GetCurrent();
  HandleScope scope(isolate);
  FtpWrapper *obj = reinterpret_cast<FtpWrapper*>(user_data);
  artik_bt_ftp_property *p = reinterpret_cast<artik_bt_ftp_property*>(data);

  if (p->name == NULL || p->file_name == NULL)
    return;

  Local<Object> js_ftp_property = Object::New(isolate);
  js_ftp_property->Set(String::NewFromUtf8(isolate, "name"),
      String::NewFromUtf8(isolate, p->name));
  js_ftp_property->Set(String::NewFromUtf8(isolate, "filename"),
      String::NewFromUtf8(isolate, p->file_name));
  js_ftp_property->Set(String::NewFromUtf8(isolate, "status"),
      String::NewFromUtf8(isolate, p->status));
  js_ftp_property->Set(String::NewFromUtf8(isolate, "transfered"),
      Integer::New(isolate, p->transfered));
  js_ftp_property->Set(String::NewFromUtf8(isolate, "size"),
      Integer::New(isolate, p->size));
  Local<Value> argv[] = {
    String::NewFromUtf8(isolate, "transfer"),
    js_ftp_property
  };

  obj->emit(isolate, obj, 2, argv);
}

FtpWrapper::FtpWrapper()
  : m_bt(new Bluetooth()) {
  m_loop = GlibLoop::Instance();
  m_loop->attach();
}

FtpWrapper::~FtpWrapper() {
  delete m_bt;
  delete m_emit;
  m_loop->detach();
}

void FtpWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  log_dbg("");

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "Ftp"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "create_session", create_session);
  NODE_SET_PROTOTYPE_METHOD(tpl, "remove_session", remove_session);
  NODE_SET_PROTOTYPE_METHOD(tpl, "change_folder", change_folder);
  NODE_SET_PROTOTYPE_METHOD(tpl, "create_folder", create_folder);
  NODE_SET_PROTOTYPE_METHOD(tpl, "delete_file", delete_file);
  NODE_SET_PROTOTYPE_METHOD(tpl, "list_folder", list_folder);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_file", get_file);
  NODE_SET_PROTOTYPE_METHOD(tpl, "put_file", put_file);
  NODE_SET_PROTOTYPE_METHOD(tpl, "resume_transfer", resume_transfer);
  NODE_SET_PROTOTYPE_METHOD(tpl, "suspend_transfer", suspend_transfer);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "Ftp"),
         tpl->GetFunction());
}

void FtpWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  log_dbg("");

  if (args.IsConstructCall()) {
    FtpWrapper* obj = new FtpWrapper();
    Bluetooth *bt = obj->getObj();
    obj->Wrap(args.This());
    Local<Function> emit =
      Local<Function>::Cast(obj->handle()->Get
          (String::NewFromUtf8(isolate, "emit")));
    obj->m_emit = new v8::Persistent<v8::Function>(isolate, emit);
    bt->set_callback(BT_EVENT_FTP, _ftp_event_callback, obj);

    args.GetReturnValue().Set(args.This());
  } else {
    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, 0, NULL).ToLocalChecked());
  }
}

void FtpWrapper::emit(Isolate *isolate, ObjectWrap* obj, int argc,
    Local<Value> argv[]) {
  Local<Function> emit_f = Local<Function>::New(isolate, *m_emit);
  node::MakeCallback(isolate, obj->handle(), emit_f, argc, argv);
}

void FtpWrapper::create_session(
    const FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  FtpWrapper *obj = ObjectWrap::Unwrap<FtpWrapper>(args.Holder());
  Bluetooth* bt = obj->getObj();

  log_dbg("");

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

  String::Utf8Value val(args[0]->ToString());
  artik_error err = bt->ftp_create_session(*val);

  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void FtpWrapper::remove_session(const FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<FtpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  artik_error err = obj->ftp_remove_session();

  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void FtpWrapper::change_folder(const FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<FtpWrapper>(args.Holder())->getObj();

  log_dbg("");

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

  String::Utf8Value val(args[0]->ToString());
  artik_error err = obj->ftp_change_folder(*val);

  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void FtpWrapper::create_folder(const FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<FtpWrapper>(args.Holder())->getObj();

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
  artik_error err = obj->ftp_create_folder(*val);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void FtpWrapper::delete_file(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<FtpWrapper>(args.Holder())->getObj();

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
  artik_error err = obj->ftp_delete_file(*val);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void FtpWrapper::list_folder(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* bt = ObjectWrap::Unwrap<FtpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  }

  artik_bt_ftp_file *head_file_list, *file_list;
  artik_error err = bt->ftp_list_folder(&head_file_list);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }

  int i = 0;
  file_list = head_file_list;
  Local<Array> js_items = Array::New(isolate);

  while (file_list != NULL) {
    Local<Object> js_item = Object::New(isolate);
    js_item->Set(
      String::NewFromUtf8(isolate, "type"),
      String::NewFromUtf8(isolate, file_list->file_type ? file_list->file_type :
                                                          " "));
    js_item->Set(
      String::NewFromUtf8(isolate, "filename"),
      String::NewFromUtf8(isolate, file_list->file_name ? file_list->file_name :
                                                          " "));
    js_item->Set(
      String::NewFromUtf8(isolate, "modified"),
      String::NewFromUtf8(isolate, file_list->modified ? file_list->modified :
                                                          " "));
    js_item->Set(
      String::NewFromUtf8(isolate, "permission"),
      String::NewFromUtf8(isolate, file_list->file_permission ?
        file_list->file_permission : " "));
    js_item->Set(
      String::NewFromUtf8(isolate, "size"),
      Integer::New(isolate, file_list->size));
    js_items->Set(i, js_item);
    i++;
    file_list = file_list->next_file;
  }

  file_list = head_file_list;
  while (file_list != NULL) {
    artik_bt_ftp_file *next = file_list->next_file;
    free(file_list->file_type);
    free(file_list->file_name);
    free(file_list->file_permission);
    free(file_list->modified);
    free(file_list);
    file_list = next;
  }

  args.GetReturnValue().Set(js_items);
}

void FtpWrapper::get_file(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<FtpWrapper>(args.Holder())->getObj();

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

  String::Utf8Value target_file(args[0]->ToString());
  String::Utf8Value source_file(args[1]->ToString());
  artik_error err = obj->ftp_get_file(*target_file, *source_file);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void FtpWrapper::put_file(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<FtpWrapper>(args.Holder())->getObj();

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

  String::Utf8Value source_file(args[0]->ToString());
  String::Utf8Value target_file(args[1]->ToString());
  artik_error err = obj->ftp_put_file(*source_file, *target_file);
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void FtpWrapper::resume_transfer(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<FtpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  }

  artik_error err = obj->ftp_resume_transfer();
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

void FtpWrapper::suspend_transfer(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Bluetooth* obj = ObjectWrap::Unwrap<FtpWrapper>(args.Holder())->getObj();

  log_dbg("");

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  }

  artik_error err = obj->ftp_suspend_transfer();
  if (err != S_OK) {
    std::string msg = "Error: " + std::string(error_msg(err));
    isolate->ThrowException(Exception::Error(
        String::NewFromUtf8(isolate, msg.c_str())));
    return;
  }
}

}  // namespace artik

