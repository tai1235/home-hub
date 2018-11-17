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

#include <artik_module.h>

#include <node.h>

#include "i2c/i2c.h"
#include "gpio/gpio.h"
#include "serial/serial.h"
#include "pwm/pwm.h"
#include "adc/adc.h"
#include "http/http.h"
#include "websocket/websocket.h"
#include "cloud/cloud.h"
#include "wifi/wifi.h"
#include "media/media.h"
#include "spi/spi.h"
#include "time/time.h"
#include "sensor/sensor.h"
#include "network/network.h"
#include "bluetooth/bluetooth.h"
#include "zigbee/zigbee.h"
#include "lwm2m/lwm2m.h"
#include "mqtt/mqtt.h"
#include "security/security.h"

namespace artik {

  using v8::Local;
  using v8::Object;
  using v8::Number;

  void GetModules(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    artik_api_module* modules = NULL;
    int num_modules = 0;
    char *modules_str = NULL;

    artik_get_available_modules(&modules, &num_modules);

    modules_str = reinterpret_cast<char*>(
        malloc(num_modules * (MAX_MODULE_NAME + 2)));
    memset(modules_str, '\0', num_modules * (MAX_MODULE_NAME + 2));

    for (int i = 0; i < num_modules; i++) {
      modules_str = strncat(modules_str, modules[i].name, MAX_MODULE_NAME);
      modules_str = strncat(modules_str, "\n", 1);
    }

    args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, modules_str));

    free(modules_str);
  }

  void GetPlatform(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    int platform = artik_get_platform();

    args.GetReturnValue().Set(Number::New(isolate, platform));
  }

  void GetPlatformName(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    char platname[MAX_PLATFORM_NAME] = "";

    artik_get_platform_name(platname);
    args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, platname));
  }

  void GetDeviceInfo(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    char* json = artik_get_device_info();

    args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, json));
    free(json);
  }

  void GetBtMacAddress(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    char addr[MAX_BT_ADDR+1] = "";

    artik_get_bt_mac_address(addr);
    args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, addr));
  }

  void GetWifiMacAddress(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    char addr[MAX_WIFI_ADDR+1] = "";

    artik_get_wifi_mac_address(addr);
    args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, addr));
  }

  void GetPlatformSN(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    char sn[MAX_PLATFORM_SN+1] = "";

    artik_get_platform_serial_number(sn);
    args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, sn));
  }

  void GetPlatformManufacturer(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    char manu[MAX_PLATFORM_MANUFACT+1] = "";

    artik_get_platform_manufacturer(manu);
    args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, manu));
  }

  void GetPlatformUptime(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    int64_t uptime = 0;

    artik_get_platform_uptime(&uptime);
    args.GetReturnValue().Set(Number::New(isolate, uptime));
  }

  void GetPlatformModelNumber(const v8::FunctionCallbackInfo<v8::Value>& args) {
    v8::Isolate* isolate = args.GetIsolate();
    char modelnum[MAX_PLATFORM_MODELNUM+1] = "";

    artik_get_platform_model_number(modelnum);
    args.GetReturnValue().Set(v8::String::NewFromUtf8(isolate, modelnum));
  }

  void DestroyAll(const v8::FunctionCallbackInfo<v8::Value>& args) {
  }

  void InitAll(Local<Object> exports) {
    /* Register own methods */
    NODE_SET_METHOD(exports, "get_modules", GetModules);
    NODE_SET_METHOD(exports, "get_platform", GetPlatform);
    NODE_SET_METHOD(exports, "get_platform_name", GetPlatformName);
    NODE_SET_METHOD(exports, "get_device_info", GetDeviceInfo);
    NODE_SET_METHOD(exports, "get_bt_mac_address", GetBtMacAddress);
    NODE_SET_METHOD(exports, "get_wifi_mac_address", GetWifiMacAddress);
    NODE_SET_METHOD(exports, "get_platform_serial_number", GetPlatformSN);
    NODE_SET_METHOD(exports, "get_platform_manufacturer", GetPlatformManufacturer);
    NODE_SET_METHOD(exports, "get_platform_uptime", GetPlatformUptime);
    NODE_SET_METHOD(exports, "get_platform_model_number", GetPlatformModelNumber);
    NODE_SET_METHOD(exports, "destroy", DestroyAll);

    /* Register all modules */
    GpioWrapper::Init(exports);
    I2cWrapper::Init(exports);
    SerialWrapper::Init(exports);
    PwmWrapper::Init(exports);
    AdcWrapper::Init(exports);
    HttpWrapper::Init(exports);
    CloudWrapper::Init(exports);
    WifiWrapper::Init(exports);
    MediaWrapper::Init(exports);
    SpiWrapper::Init(exports);
    TimeWrapper::Init(exports);
    SensorWrapper::Init(exports);
    NetworkWrapper::Init(exports);
    WebsocketWrapper::Init(exports);
    BluetoothWrapper::Init(exports);
    ZigbeeWrapper::Init(exports);
    Lwm2mWrapper::Init(exports);
    MqttWrapper::Init(exports);
    SecurityWrapper::Init(exports);
  }

  NODE_MODULE(artik, InitAll)

}  // namespace artik
