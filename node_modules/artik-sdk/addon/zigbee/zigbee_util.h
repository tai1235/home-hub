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

#ifndef ADDON_ZIGBEE_ZIGBEE_UTIL_H_
#define ADDON_ZIGBEE_ZIGBEE_UTIL_H_

using v8::Function;
using v8::Local;
using v8::Isolate;
using v8::Object;

namespace artik {

void zb_callback(void *user_data, artik_zigbee_response_type response_type,
                 void *payload);
char* convert_endpointlist_to_json(const artik_zigbee_endpoint_list *list);
char *convert_device_info(const artik_zigbee_device_info *di);
int convert_jsobject_levelcontrol(Isolate* isolate, const Local<Object>& in,
                                  artik_zigbee_level_control_command *out);
int convert_jsobject_endpoint(Isolate* isolate, const Local<Object>& in,
                              artik_zigbee_endpoint *out);
void throw_error(Isolate *isolate, artik_error code);

}  // namespace artik

#endif  // ADDON_ZIGBEE_ZIGBEE_UTIL_H_
