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

#include "serial/serial.h"

#include <unistd.h>
#include <node_buffer.h>
#include <nan.h>
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
using v8::Handle;
using v8::Context;

Persistent<Function> SerialWrapper::constructor;

const std::array<int, 7> SerialWrapper::s_baudrates = {
  4800, 9600, 14400, 19200, 38400, 57600, 115200
};

const std::array<const char *, 3> SerialWrapper::s_parities = {
  "none", "odd", "even"
};

const std::array<int, 2> SerialWrapper::s_data_bits = {
  7, 8
};

const std::array<int, 2> SerialWrapper::s_stop_bits = {
  1, 2
};

const std::array<const char*, 3> SerialWrapper::s_flowcontrols = {
  "none", "hard", "soft"
};

static void serial_change_callback(void *user_data, unsigned char *buf,
    int len) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  SerialWrapper *wrap = reinterpret_cast<SerialWrapper*>(user_data);

  if (!wrap->getChangeCb() || !buf)
    return;

  v8::MaybeLocal<v8::Object> array = node::Buffer::Copy(isolate,
      reinterpret_cast<char*>(buf), len);

  Handle<Value> argv[] = {
    array.ToLocalChecked()
  };

  Local<Function>::New(isolate, *wrap->getChangeCb())->Call(
      isolate->GetCurrentContext()->Global(), 1, argv);
}

SerialWrapper::SerialWrapper(unsigned int port, char *name,
    artik_serial_baudrate_t baudrate, artik_serial_parity_t parity,
    artik_serial_data_bits_t data, artik_serial_stop_bits_t stop,
    artik_serial_flowcontrol_t flowctrl) {
  m_serial = new Serial(port, name, baudrate, parity, data, stop, flowctrl);
  m_rx_buf_size = 128;
  m_loop = GlibLoop::Instance();
  m_loop->attach();
}

SerialWrapper::~SerialWrapper() {
  delete m_serial;
  m_loop->detach();
}

void SerialWrapper::Init(Local<Object> exports) {
  static_assert(s_baudrates.size() == ARTIK_SERIAL_BAUD_115200 + 1,
      "Bad size");
  static_assert(s_parities.size() == ARTIK_SERIAL_PARITY_EVEN + 1,
      "Bad size");
  static_assert(s_data_bits.size() == ARTIK_SERIAL_DATA_8BIT + 1, "Bad size");
  static_assert(s_stop_bits.size() == ARTIK_SERIAL_STOP_2BIT + 1, "Bad size");
  static_assert(s_flowcontrols.size() == ARTIK_SERIAL_FLOWCTRL_SOFT + 1,
      "Bad size");

  Isolate* isolate = exports->GetIsolate();
  Local<FunctionTemplate> modal = FunctionTemplate::New(isolate, New);

  modal->SetClassName(String::NewFromUtf8(isolate, "serial"));
  modal->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(modal, "request", request);
  NODE_SET_PROTOTYPE_METHOD(modal, "release", release);
  NODE_SET_PROTOTYPE_METHOD(modal, "write", write);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_port_num", get_port_num);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_name", get_name);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_baudrate", get_baudrate);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_parity", get_parity);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_data_bits", get_data_bits);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_stop_bits", get_stop_bits);
  NODE_SET_PROTOTYPE_METHOD(modal, "get_flowctrl", get_flowctrl);

  NODE_SET_PROTOTYPE_METHOD(modal, "set_port_num", set_port_num);
  NODE_SET_PROTOTYPE_METHOD(modal, "set_name", set_name);
  NODE_SET_PROTOTYPE_METHOD(modal, "set_baudrate", set_baudrate);
  NODE_SET_PROTOTYPE_METHOD(modal, "set_parity", set_parity);
  NODE_SET_PROTOTYPE_METHOD(modal, "set_data_bits", set_data_bits);
  NODE_SET_PROTOTYPE_METHOD(modal, "set_stop_bits", set_stop_bits);
  NODE_SET_PROTOTYPE_METHOD(modal, "set_flowctrl", set_flowctrl);

  constructor.Reset(isolate, modal->GetFunction());
  exports->Set(v8::String::NewFromUtf8(isolate, "serial"),
      modal->GetFunction());
}

void SerialWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate *isolate = args.GetIsolate();
  SerialWrapper* obj = NULL;
  int lenArg = 7;

  if (args.Length() != lenArg && args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }
  if (args.IsConstructCall()) {
    if (args.Length() == lenArg) {
        v8::String::Utf8Value param1(args[1]->ToString());
        char* name = *param1;

        auto baudrate = to_artik_parameter<artik_serial_baudrate_t>(s_baudrates,
            args[2]->NumberValue());
        if (!baudrate) {
          std::string error = "Baudrate " +
            std::to_string(args[2]->NumberValue()) + " is not supported";
          isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
              isolate, error.c_str())));
          return;
        }

        v8::String::Utf8Value param3(args[3]->ToString());
        auto parity = to_artik_parameter<artik_serial_parity_t>(s_parities,
            *param3);
        if (!parity) {
          std::string error = "Parity " + std::string(*param3) +
            " is not supported";
          isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
              isolate, error.c_str())));
          return;
        }

        auto data_bits = to_artik_parameter<artik_serial_data_bits_t>(
            s_data_bits, args[4]->NumberValue());
        if (!data_bits) {
          std::string error = "Data bit length " +
              std::to_string(args[4]->NumberValue()) + " is not supported";
          isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
              isolate, error.c_str())));
          return;
        }

        auto stop_bit = to_artik_parameter<artik_serial_stop_bits_t>(
            s_stop_bits, args[5]->NumberValue());
        if (!stop_bit) {
          std::string error = "Stop bit " +
              std::to_string(args[5]->Int32Value()) + " is not supported";
          isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
              isolate, error.c_str())));
          return;
        }

        v8::String::Utf8Value param6(args[6]->ToString());
        auto flowcontrol = to_artik_parameter<artik_serial_flowcontrol_t>(
            s_flowcontrols, *param6);
        if (!flowcontrol) {
          std::string error = "Flowcontrol " + std::string(*param6) +
               " is not supported";
          isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
                                            isolate, error.c_str())));
          return;
        }

        obj = new SerialWrapper(
            args[0]->Uint32Value(),
            name,
            baudrate.value(),
            parity.value(),
            data_bits.value(),
            stop_bit.value(),
            (artik_serial_flowcontrol_t)args[6]->Int32Value());
    } else {
        obj = new SerialWrapper();
    }

    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    Local<Value> argv[lenArg];

    if (args.Length() == lenArg) {
      for (int i = 0; i < lenArg; ++i)
        argv[i] = args[i];
    }

    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, args.Length(), argv).ToLocalChecked());
  }
}

void SerialWrapper::request(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() > 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  SerialWrapper *wrap = ObjectWrap::Unwrap<SerialWrapper>(args.Holder());
  Serial* obj = wrap->getObj();
  artik_error ret = obj->request();
  if ((ret == S_OK) && args[0]->IsFunction()) {
    wrap->m_change_cb = new v8::Persistent<v8::Function>();
    wrap->m_change_cb->Reset(isolate, Local<Function>::Cast(args[0]));
    obj->set_received_callback(serial_change_callback,
        reinterpret_cast<void*>(wrap));
  }
  args.GetReturnValue().Set(Number::New(isolate, ret));
}

void SerialWrapper::release(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  SerialWrapper* wrap = ObjectWrap::Unwrap<SerialWrapper>(args.Holder());
  Serial* obj = wrap->getObj();
  if (wrap->m_change_cb) {
    obj->unset_received_callback();
    delete wrap->m_change_cb;
    wrap->m_change_cb = NULL;
  }
  args.GetReturnValue().Set(Number::New(isolate, obj->release()));
}

void SerialWrapper::write(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  Serial* obj = ObjectWrap::Unwrap<SerialWrapper>(args.Holder())->getObj();

  if ((args.Length() < 1)) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong arguments")));
    return;
  }

  if (!node::Buffer::HasInstance(args[0])) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Argument should be a Buffer.")));
    return;
  }

  unsigned char *buffer = (unsigned char*)node::Buffer::Data(args[0]);
  int length = static_cast<int>(node::Buffer::Length(args[0]));

  int ret = obj->write(buffer, &length);

  args.GetReturnValue().Set(Number::New(isolate, (ret == S_OK) ? length : 0));
}

void SerialWrapper::get_port_num(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  if (args.Length() != 0)
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
  Serial* obj = ObjectWrap::Unwrap<SerialWrapper>(args.Holder())->getObj();

  args.GetReturnValue().Set(Number::New(isolate,
        static_cast<int>(obj->get_port_num())));
}

void SerialWrapper::get_name(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  Serial* obj = ObjectWrap::Unwrap<SerialWrapper>(args.Holder())->getObj();

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, obj->get_name()));
}

void SerialWrapper::get_baudrate(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  Serial* obj = ObjectWrap::Unwrap<SerialWrapper>(args.Holder())->getObj();

  int baudrate = s_baudrates[obj->get_baudrate()];
  args.GetReturnValue().Set(Number::New(isolate, baudrate));
}

void SerialWrapper::get_parity(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  Serial* obj = ObjectWrap::Unwrap<SerialWrapper>(args.Holder())->getObj();
  const char* parity = s_parities[obj->get_parity()];

  args.GetReturnValue().Set(String::NewFromUtf8(isolate, parity));
}

void SerialWrapper::get_data_bits(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  Serial* obj = ObjectWrap::Unwrap<SerialWrapper>(args.Holder())->getObj();

  int data_bits = s_data_bits[obj->get_data_bits()];
  args.GetReturnValue().Set(Number::New(isolate, data_bits));
}

void SerialWrapper::get_stop_bits(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  Serial* obj = ObjectWrap::Unwrap<SerialWrapper>(args.Holder())->getObj();

  int stop_bit = s_stop_bits[obj->get_stop_bits()];
  args.GetReturnValue().Set(Number::New(isolate, stop_bit));
}

void SerialWrapper::get_flowctrl(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  Serial* obj = ObjectWrap::Unwrap<SerialWrapper>(args.Holder())->getObj();
  const char * flowcontrol = s_flowcontrols[obj->get_flowctrl()];
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, flowcontrol));
}

void SerialWrapper::set_port_num(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  Serial* obj = ObjectWrap::Unwrap<SerialWrapper>(args.Holder())->getObj();

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  obj->set_port_num(static_cast<int>(args[0]->NumberValue()));
}

void SerialWrapper::set_name(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  Serial* obj = ObjectWrap::Unwrap<SerialWrapper>(args.Holder())->getObj();
  v8::String::Utf8Value val(args[0]->ToString());

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  obj->set_name(*val);
}

void SerialWrapper::set_baudrate(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  Serial* obj = ObjectWrap::Unwrap<SerialWrapper>(args.Holder())->getObj();

  auto baudrate = to_artik_parameter<artik_serial_baudrate_t>(s_baudrates,
      args[0]->NumberValue());
  if (!baudrate) {
    std::string error = "Baudrate " + std::to_string(args[0]->NumberValue()) +
        " is not supported";
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
                                     isolate, error.c_str())));
    return;
  }
  obj->set_baudrate(baudrate.value());
}

void SerialWrapper::set_parity(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  v8::String::Utf8Value param0(args[0]->ToString());
  auto parity = to_artik_parameter<artik_serial_parity_t>(s_parities, *param0);
  if (!parity) {
    std::string error = "Parity " + std::string(*param0) + " is not supported";
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, error.c_str())));
    return;
  }
  Serial* obj = ObjectWrap::Unwrap<SerialWrapper>(args.Holder())->getObj();

  obj->set_parity(parity.value());
}

void SerialWrapper::set_data_bits(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  }

  Serial* obj = ObjectWrap::Unwrap<SerialWrapper>(args.Holder())->getObj();

  auto data_bits = to_artik_parameter<artik_serial_data_bits_t>(s_data_bits,
      args[0]->NumberValue());
  if (!data_bits) {
    std::string error = "Data bit length " +
        std::to_string(args[0]->NumberValue()) + " is not supported";
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, error.c_str())));
    return;
  }
  obj->set_data_bits(data_bits.value());
}

void SerialWrapper::set_stop_bits(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  Serial* obj = ObjectWrap::Unwrap<SerialWrapper>(args.Holder())->getObj();

  auto stop_bit = to_artik_parameter<artik_serial_stop_bits_t>(s_stop_bits,
      args[0]->NumberValue());
  if (!stop_bit) {
    std::string error = "Stop bit " + std::to_string(args[0]->NumberValue()) +
        " is not supported";
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, error.c_str())));
    return;
  }

  obj->set_stop_bits(stop_bit.value());
}

void SerialWrapper::set_flowctrl(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  Serial* obj = ObjectWrap::Unwrap<SerialWrapper>(args.Holder())->getObj();

  v8::String::Utf8Value param0(args[0]->ToString());
  auto flowcontrol = to_artik_parameter<artik_serial_flowcontrol_t>(
      s_flowcontrols, *param0);
  if (!flowcontrol) {
    std::string error = "Flowcontrol " + std::string(*param0) +
        " is not supported";
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, error.c_str())));
    return;
  }
  obj->set_flowctrl(flowcontrol.value());
}

}  // namespace artik

