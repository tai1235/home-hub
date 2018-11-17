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

#include "network/network.h"

#include <unistd.h>
#include <stdlib.h>
#include <artik_log.h>

#include <string>
#include <vector>

namespace artik {

using v8::Exception;
using v8::Function;
using v8::FunctionCallbackInfo;
using v8::FunctionTemplate;
using v8::Isolate;
using v8::Local;
using v8::Number;
using v8::Boolean;
using v8::Object;
using v8::Persistent;
using v8::String;
using v8::Value;
using v8::Array;
using v8::Handle;
using v8::Int32;
using v8::Context;

Persistent<Function> NetworkWrapper::constructor;

static Local<Object> network_config_object(Isolate *isolate,
    artik_network_config *config) {
  Local<Array> dnsAddr = Array::New(isolate);

  Local<Object> js_property = Object::New(isolate);
  js_property->Set(String::NewFromUtf8(isolate, "ip_addr"),
      String::NewFromUtf8(isolate, config->ip_addr.address));

  js_property->Set(String::NewFromUtf8(isolate, "netmask"),
      String::NewFromUtf8(isolate, config->netmask.address));

  js_property->Set(String::NewFromUtf8(isolate, "gw_addr"),
      String::NewFromUtf8(isolate, config->gw_addr.address));

  for (int i = 0; i < MAX_DNS_ADDRESSES; i++)
    dnsAddr->Set(i, String::NewFromUtf8(isolate, config->dns_addr[i].address));

  js_property->Set(String::NewFromUtf8(isolate, "dns_addr"),
      dnsAddr);

  js_property->Set(String::NewFromUtf8(isolate, "mac_addr"),
      String::NewFromUtf8(isolate,
        reinterpret_cast<const char*>(config->mac_addr)));

  return js_property;
}

static bool updateNetworkConfig(Isolate *isolate, Local<Value> val,
    artik_network_config *net_config) {

  // ip_addr parameter
  auto ip_addr = js_object_attribute_to_cpp<std::string>(val, "ip_addr");

  if (ip_addr) {
    strncpy(net_config->ip_addr.address, ip_addr.value().c_str(),
        MAX_IP_ADDRESS_LEN);
  } else {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "ip_addr undefined")));
    return false;
  }

  // netmask parameter
  auto netmask = js_object_attribute_to_cpp<std::string>(val, "netmask");

  if (netmask) {
    strncpy(net_config->netmask.address, netmask.value().c_str(),
        MAX_IP_ADDRESS_LEN);
  } else {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "netmask undefined")));
    return false;
  }

  // gw_addr parameter
  auto gw_addr = js_object_attribute_to_cpp<std::string>(val, "gw_addr");

  if (gw_addr) {
    strncpy(net_config->gw_addr.address, gw_addr.value().c_str(),
        MAX_IP_ADDRESS_LEN);
  } else {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "gw_addr undefined")));
    return false;
  }

  // dns_addr parameter
  auto dns_addr = js_object_attribute_to_cpp<std::vector<std::string>>(val,
      "dns_addr");

  if (dns_addr) {
    for (int i = 0; i < MAX_DNS_ADDRESSES; i++)
      strncpy(net_config->dns_addr[i].address, "", 2);

    if ((dns_addr.value().size() <= MAX_DNS_ADDRESSES) &&
        (dns_addr.value().size() > 0)) {
      for (unsigned int i = 0; i < dns_addr.value().size(); i++)
        strncpy(net_config->dns_addr[i].address,
            dns_addr.value().at(i).c_str(), MAX_IP_ADDRESS_LEN);
    } else {
      isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, "dns_addr cannot contain more than "
          "two DNS addresses")));
      return false;
    }
  } else {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "dns_addr undefined")));
    return false;
  }

  return true;
}

static bool updateDHCPServerConfig(Isolate *isolate, Local<Value> val,
    artik_network_dhcp_server_config *dhcp_server_config) {

  // iface parameter
  auto interface = js_object_attribute_to_cpp<std::string>(val, "iface");

  if (interface) {
    if (interface.value() == "wifi") {
      dhcp_server_config->interface = ARTIK_WIFI;
    } else if (interface.value() == "ethernet") {
      dhcp_server_config->interface = ARTIK_ETHERNET;
    } else {
      isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, "Wrong definition of interface : "
              "expect 'wifi' or 'ethernet'")));
      return false;
    }
  } else {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "iface undefined")));
    return false;
  }

  // ip_addr parameter
  auto ip_addr = js_object_attribute_to_cpp<std::string>(val, "ip_addr");

  if (ip_addr) {
    strncpy(dhcp_server_config->ip_addr.address, ip_addr.value().c_str(),
        MAX_IP_ADDRESS_LEN);
  } else {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "ip_addr undefined")));
    return false;
  }

  // netmask parameter
  auto netmask = js_object_attribute_to_cpp<std::string>(val, "netmask");

  if (netmask) {
    strncpy(dhcp_server_config->netmask.address, netmask.value().c_str(),
        MAX_IP_ADDRESS_LEN);
  } else {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "netmask undefined")));
    return false;
  }

  // gw_addr parameter
  auto gw_addr = js_object_attribute_to_cpp<std::string>(val, "gw_addr");

  if (gw_addr) {
    strncpy(dhcp_server_config->gw_addr.address, gw_addr.value().c_str(),
        MAX_IP_ADDRESS_LEN);
  } else {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "gw_addr undefined")));
    return false;
  }

  // dns_addr parameter
  auto dns_addr = js_object_attribute_to_cpp<std::vector<std::string>>(val,
      "dns_addr");

  if (dns_addr) {
    for (int i = 0; i < MAX_DNS_ADDRESSES; i++)
      strncpy(dhcp_server_config->dns_addr[i].address, "", 2);

    if ((dns_addr.value().size() <= MAX_DNS_ADDRESSES) &&
        (dns_addr.value().size() > 0)) {
      for (unsigned int i = 0; i < dns_addr.value().size(); i++)
        strncpy(dhcp_server_config->dns_addr[i].address,
            dns_addr.value().at(i).c_str(), MAX_IP_ADDRESS_LEN);
    } else {
      isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, "dns_addr cannot contain more than "
          "two DNS addresses")));
      return false;
    }
  } else {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "dns_addr undefined")));
    return false;
  }

  // start_addr parameter
  auto start_addr = js_object_attribute_to_cpp<std::string>(val, "start_addr");

  if (start_addr) {
    strncpy(dhcp_server_config->start_addr.address,
        start_addr.value().c_str(), MAX_IP_ADDRESS_LEN);
  } else {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "start_addr undefined")));
    return false;
  }

  // num_leases parameter
  auto num_leases = js_object_attribute_to_cpp<unsigned int>(val, "num_leases");

  if (num_leases) {
    dhcp_server_config->num_leases = num_leases.value();
  } else {
    isolate->ThrowException(Exception::TypeError(
        String::NewFromUtf8(isolate, "num_leases undefined")));
    return false;
  }

  return true;
}

static void watch_online_status_callback(bool network_status,
    void *user_data) {
  Isolate *isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  NetworkWrapper *wrap = reinterpret_cast<NetworkWrapper*>(user_data);
  Handle<Value> argv[] = { Boolean::New(isolate, network_status) };

  Local<Function>::New(isolate,
      *wrap->getWatchOnlineStatusCb())->Call(
          isolate->GetCurrentContext()->Global(), 1, argv);
}

NetworkWrapper::NetworkWrapper(v8::Persistent<v8::Function> *callback,
    bool enable_watch_online_status) {
  m_handle = NULL;
  m_network = new Network();
  m_loop = GlibLoop::Instance();
  m_loop->attach();

  if (enable_watch_online_status) {
    m_watch_online_status_cb = callback;
    m_network->add_watch_online_status(&m_handle,
        watch_online_status_callback, this);
  }
}

NetworkWrapper::~NetworkWrapper() {
  if (m_handle != NULL && m_watch_online_status_cb != NULL) {
    m_network->remove_watch_online_status(m_handle);
  }

  delete m_network;
  if (m_watch_online_status_cb != NULL)
    delete m_watch_online_status_cb;

  m_loop->detach();
}

void NetworkWrapper::Init(Local<Object> exports) {
  Isolate* isolate = exports->GetIsolate();

  // Prepare constructor template
  Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
  tpl->SetClassName(String::NewFromUtf8(isolate, "network"));
  tpl->InstanceTemplate()->SetInternalFieldCount(1);

  // Prototypes
  NODE_SET_PROTOTYPE_METHOD(tpl, "set_network_config",
      set_network_config);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_network_config",
      get_network_config);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_current_public_ip",
      get_current_public_ip);
  NODE_SET_PROTOTYPE_METHOD(tpl, "dhcp_client_start", dhcp_client_start);
  NODE_SET_PROTOTYPE_METHOD(tpl, "dhcp_client_stop", dhcp_client_stop);
  NODE_SET_PROTOTYPE_METHOD(tpl, "dhcp_server_start", dhcp_server_start);
  NODE_SET_PROTOTYPE_METHOD(tpl, "dhcp_server_stop", dhcp_server_stop);
  NODE_SET_PROTOTYPE_METHOD(tpl, "get_online_status", get_online_status);

  constructor.Reset(isolate, tpl->GetFunction());
  exports->Set(String::NewFromUtf8(isolate, "network"),
               tpl->GetFunction());
}

void NetworkWrapper::New(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  bool enable_watch_online_status = false;

  if (args.Length() > 3 || args.Length() < 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (args[1]->IsBoolean())
        enable_watch_online_status = args[1]->BooleanValue();

  if (args.IsConstructCall()) {
    NetworkWrapper* obj = new NetworkWrapper(
        new v8::Persistent<v8::Function>(isolate,
            Local<Function>::Cast(args[0])), enable_watch_online_status);
    obj->Wrap(args.This());
    args.GetReturnValue().Set(args.This());
  } else {
    int argc = args.Length();
    Local<Value> argv[argc];

    for (int i = 0; i < argc; i++)
      argv[i] = args[i];

    Local<Context> context = isolate->GetCurrentContext();
    Local<Function> cons = Local<Function>::New(isolate, constructor);
    args.GetReturnValue().Set(
        cons->NewInstance(context, argc, argv).ToLocalChecked());
  }
}

void NetworkWrapper::set_network_config(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  NetworkWrapper* wrap = ObjectWrap::Unwrap<NetworkWrapper>(args.Holder());
  Network* network = wrap->getObj();
  artik_network_interface_t interface = ARTIK_WIFI;
  artik_network_config config;

  if (args.Length() != 2) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  }

  memset(&config, 0, sizeof(config));

  if (args[0]->IsObject()) {
    if (!updateNetworkConfig(isolate, args[0], &config))
      return;
  }

  if (args[1]->IsString()) {
    auto connection_type = js_type_to_cpp<std::string>(args[1]);

    if (connection_type.value() == "wifi") {
      interface = ARTIK_WIFI;
    } else if (connection_type.value() == "ethernet") {
      interface = ARTIK_ETHERNET;
    } else {
      isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, "Wrong definition of connection_type :"
                                      " expect 'wifi' or 'ethernet'.")));
      return;
    }
  }

  artik_error ret = network->set_network_config(&config, interface);

  if (ret != S_OK) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Failed to set network config")));
      return;
  }

  args.GetReturnValue().Set(Number::New(isolate, ret));
}

void NetworkWrapper::get_network_config(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  NetworkWrapper* wrap = ObjectWrap::Unwrap<NetworkWrapper>(args.Holder());
  Network* network = wrap->getObj();
  artik_network_interface_t interface = ARTIK_WIFI;

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  }

  if (args[0]->IsString()) {
    auto connection_type = js_type_to_cpp<std::string>(args[0]);

    if (connection_type.value() == "wifi") {
      interface = ARTIK_WIFI;
    } else if (connection_type.value() == "ethernet") {
      interface = ARTIK_ETHERNET;
    } else {
      isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, "Wrong definition of connection_type :"
                                      " expect 'wifi' or 'ethernet'.")));
      return;
    }
  }

  artik_network_config config = {};

  artik_error ret = network->get_network_config(&config, interface);

  if (ret != S_OK) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Failed to get current network config")));
      return;
  }

  Local<Object> js_property = network_config_object(isolate, &config);

  args.GetReturnValue().Set(js_property);
}

void NetworkWrapper::get_current_public_ip(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  NetworkWrapper* wrap = ObjectWrap::Unwrap<NetworkWrapper>(args.Holder());
  Network* network = wrap->getObj();
  artik_network_ip *current_ip = reinterpret_cast<artik_network_ip*>(malloc(
      sizeof(artik_network_ip)));

  if (network->get_current_public_ip(current_ip) == S_OK) {
    args.GetReturnValue().Set(String::NewFromUtf8(isolate,
        current_ip->address));
  } else {
    args.GetReturnValue().Set(String::NewFromUtf8(isolate,
        "get_current_public_ip error"));
  }

  free(current_ip);
}

void NetworkWrapper::dhcp_client_start(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  NetworkWrapper* wrap = ObjectWrap::Unwrap<NetworkWrapper>(args.Holder());
  Network* network = wrap->getObj();
  artik_network_interface_t interface = ARTIK_WIFI;

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (args[0]->IsString()) {
    auto connection_type = js_type_to_cpp<std::string>(args[0]);

    if (connection_type.value() == "wifi") {
      interface = ARTIK_WIFI;
    } else if (connection_type.value() == "ethernet") {
      interface = ARTIK_ETHERNET;
    } else {
      isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, "Wrong definition of connection_type :"
                                      " expect 'wifi' or 'ethernet'.")));
      return;
    }
  }

  artik_error ret = network->dhcp_client_start(interface);

  if (ret != S_OK) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Failed to start DHCP client")));
      return;
  }

  args.GetReturnValue().Set(Number::New(isolate, ret));
}

void NetworkWrapper::dhcp_client_stop(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  NetworkWrapper* wrap = ObjectWrap::Unwrap<NetworkWrapper>(args.Holder());
  Network* network = wrap->getObj();

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Expect no argument")));
    return;
  }

  artik_error ret = network->dhcp_client_stop();

  if (ret != S_OK) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Failed to stop DHCP client")));
    return;
  }

  args.GetReturnValue().Set(Number::New(isolate, ret));
}

void NetworkWrapper::dhcp_server_start(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  NetworkWrapper* wrap = ObjectWrap::Unwrap<NetworkWrapper>(args.Holder());
  Network* network = wrap->getObj();
  artik_network_dhcp_server_config dhcp_server_config;

  memset(&dhcp_server_config, 0, sizeof(artik_network_dhcp_server_config));

  if (args.Length() != 1) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Wrong number of arguments")));
    return;
  }

  if (args[0]->IsObject()) {
    if (!updateDHCPServerConfig(isolate, args[0], &dhcp_server_config))
      return;
  }

  artik_error ret = network->dhcp_server_start(&dhcp_server_config);

  if (ret != S_OK) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Failed to start DHCP server")));
    return;
  }

  args.GetReturnValue().Set(Number::New(isolate, ret));
}

void NetworkWrapper::dhcp_server_stop(
    const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  NetworkWrapper* wrap = ObjectWrap::Unwrap<NetworkWrapper>(args.Holder());
  Network* network = wrap->getObj();

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Expect no argument")));
    return;
  }

  artik_error ret = network->dhcp_server_stop();

  if (ret != S_OK) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
        isolate, "Failed to stop DHCP server")));
    return;
  }

  args.GetReturnValue().Set(Number::New(isolate, ret));
}

void NetworkWrapper::get_online_status(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  Isolate* isolate = args.GetIsolate();
  NetworkWrapper* wrap = ObjectWrap::Unwrap<NetworkWrapper>(args.Holder());
  Network* obj = wrap->getObj();

  if (args.Length() != 0) {
    isolate->ThrowException(Exception::TypeError(String::NewFromUtf8(
      isolate, "Wrong number of arguments")));
    return;
  }

  bool online_status = false;
  obj->get_online_status(&online_status);

  args.GetReturnValue().Set(Boolean::New(isolate, online_status));
}

}  // namespace artik
