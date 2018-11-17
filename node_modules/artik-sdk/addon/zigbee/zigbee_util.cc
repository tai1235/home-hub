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

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <artik_log.h>
#include <glib.h>

#include "zigbee/zigbee.h"
#include "zigbee/zigbee_util.h"

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

typedef char* (*converter_func)(const void *payload);

static char *_convert_eui64(const char *eui64) {
  return g_strdup_printf("%02x%02x%02x%02x%02x%02x%02x%02x",
                         eui64[0], eui64[1], eui64[2], eui64[3],
                        eui64[4], eui64[5], eui64[6], eui64[7]);
}

static char *_convert_int_list(int count, const int *list) {
  int i;
  gchar **str_array = g_new0(gchar *, count + 1);
  gchar *tmp, *result;

  for (i = 0; i < count; i++) {
    str_array[i] = g_strdup_printf("%d", list[i]);
  }

  tmp = g_strjoinv(",", str_array);
  g_strfreev(str_array);

  result = g_strdup_printf("[%s]", tmp);
  g_free(tmp);

  return result;
}

static char *_convert_char_list(int count, const char *list) {
  int i;
  gchar **str_array = g_new0(gchar *, count + 1);
  gchar *tmp, *result;

  for (i = 0; i < count; i++) {
    str_array[i] = g_strdup_printf("%d", list[i]);
  }

  tmp = g_strjoinv(",", str_array);
  g_strfreev(str_array);

  result = g_strdup_printf("[%s]", tmp);
  g_free(tmp);

  return result;
}

static char* _convert_endpointlist_to_json_full(int count,
    const artik_zigbee_endpoint *list) {
  const char *tpl = "{ "
      "\"endpoint_id\": %d, "
      "\"node_id\": %d, "
      "\"server_cluster\": [%d,%d,%d,%d,%d,%d,%d,%d,%d], "
      "\"client_cluster\": [%d,%d,%d,%d,%d,%d,%d,%d,%d] "
      " }";
  gchar **str_array = g_new0(gchar *, count + 1);
  int i;
  char *tmp;

  for (i = 0; i < count; i++) {
    str_array[i] = g_strdup_printf(tpl,
        list[i].endpoint_id, list[i].node_id,
        list[i].server_cluster[0], list[i].server_cluster[1],
        list[i].server_cluster[2], list[i].server_cluster[3],
        list[i].server_cluster[4], list[i].server_cluster[5],
        list[i].server_cluster[6], list[i].server_cluster[7],
        list[i].server_cluster[8], list[i].client_cluster[0],
        list[i].client_cluster[1], list[i].client_cluster[2],
        list[i].client_cluster[3], list[i].client_cluster[4],
        list[i].client_cluster[5], list[i].client_cluster[6],
        list[i].client_cluster[7], list[i].client_cluster[8]);
  }

  tmp = g_strjoinv(",", str_array);
  g_strfreev(str_array);

  return g_strdup_printf("[%s]", tmp);
}

static char *_convert_notification(const void *payload) {
  const char *tpl = "{ "
      "\"type\": \"notification\", "
      "\"command\": \"%s\""
      " }";
  const artik_zigbee_notification notification =
      *(reinterpret_cast<const artik_zigbee_notification *>(payload));

  log_dbg("- notification: %d", notification);

  switch (notification) {
  case ARTIK_ZIGBEE_CMD_SUCCESS:
    return g_strdup_printf(tpl, "success");
    break;
  case ARTIK_ZIGBEE_CMD_ERR_PORT_PROBLEM:
    return g_strdup_printf(tpl, "port problem");
    break;
  case ARTIK_ZIGBEE_CMD_ERR_NO_SUCH_COMMAND:
    return g_strdup_printf(tpl, "no such command");
    break;
  case ARTIK_ZIGBEE_CMD_ERR_WRONG_NUMBER_OF_ARGUMENTS:
    return g_strdup_printf(tpl, "wrong number of arguments");
    break;
  case ARTIK_ZIGBEE_CMD_ERR_ARGUMENT_OUT_OF_RANGE:
    return g_strdup_printf(tpl, "argument out of range");
    break;
  case ARTIK_ZIGBEE_CMD_ERR_ARGUMENT_SYNTAX_ERROR:
    return g_strdup_printf(tpl, "argument syntax error");
    break;
  case ARTIK_ZIGBEE_CMD_ERR_STRING_TOO_LONG:
    return g_strdup_printf(tpl, "string too long");
    break;
  case ARTIK_ZIGBEE_CMD_ERR_INVALID_ARGUMENT_TYPE:
    return g_strdup_printf(tpl, "invalid argument type");
    break;
  case ARTIK_ZIGBEE_CMD_ERR:
    return g_strdup_printf(tpl, "error");
    break;
  default:
    log_err("unknown notification(%d)", notification);
    return g_strdup_printf(tpl, "unknown");
    break;
  }
}

static char *_convert_network_notification(const void *payload) {
  const char *tpl = "{ "
      "\"type\": \"network_notification\", "
      "\"status\": \"%s\""
      " }";
  const artik_zigbee_network_notification network_notification =
      *(reinterpret_cast<const artik_zigbee_network_notification *>(payload));

  log_dbg("- network_notification: %d", network_notification);

  switch (network_notification) {
  case ARTIK_ZIGBEE_NETWORK_JOIN:
    return g_strdup_printf(tpl, "join");
    break;
  case ARTIK_ZIGBEE_NETWORK_LEAVE:
    return g_strdup_printf(tpl, "leave");
    break;
  case ARTIK_ZIGBEE_NETWORK_FIND_FORM_SUCCESS:
    return g_strdup_printf(tpl, "find_form");
    break;
  case ARTIK_ZIGBEE_NETWORK_FIND_FORM_FAILED:
    return g_strdup_printf(tpl, "find_form_failed");
    break;
  case ARTIK_ZIGBEE_NETWORK_FIND_JOIN_SUCCESS:
    return g_strdup_printf(tpl, "find_join");
    break;
  case ARTIK_ZIGBEE_NETWORK_FIND_JOIN_FAILED:
    return g_strdup_printf(tpl, "find_join_failed");
    break;
  case ARTIK_ZIGBEE_NETWORK_EXIST:
    return g_strdup_printf(tpl, "network_exist");
  default:
    log_err("unknown network notification(%d)", network_notification);
    return g_strdup_printf(tpl, "unknown");
    break;
  }
}

static char *_convert_device(const artik_zigbee_device *dev) {
  const char *tpl_dev = "{ "
      "\"eui64\": \"%s\", "
      "\"node_id\": %d, "
      "\"endpoints\": %s "
      " }";
  char *str_endpoints;
  char *tmp;
  char *eui64;

  str_endpoints = _convert_endpointlist_to_json_full(dev->endpoint_count,
      dev->endpoint);
  eui64 = _convert_eui64(dev->eui64);

  tmp = g_strdup_printf(tpl_dev, eui64, dev->node_id, str_endpoints);

  g_free(str_endpoints);
  g_free(eui64);

  return tmp;
}

static char *_convert_device_discover(const void *payload) {
  const char *tpl = "{ "
      "\"type\": \"device_discover\", "
      "\"status\": \"%s\""
      " }";
  const char *tpl_dev = "{ "
      "\"type\": \"device_discover\", "
      "\"status\": \"%s\", "
      "\"device\": %s"
      " }";
  const artik_zigbee_device_discovery *device_discovery =
      reinterpret_cast<const artik_zigbee_device_discovery *>(payload);

  switch (device_discovery->status) {
  case ARTIK_ZIGBEE_DEVICE_DISCOVERY_START:
    return g_strdup_printf(tpl, "start");
    break;
  case ARTIK_ZIGBEE_DEVICE_DISCOVERY_FOUND:
    return g_strdup_printf(tpl_dev, "found",
        _convert_device(&(device_discovery->device)));
    break;
  case ARTIK_ZIGBEE_DEVICE_DISCOVERY_IN_PROGRESS:
    return g_strdup_printf(tpl, "in_progress");
    break;
  case ARTIK_ZIGBEE_DEVICE_DISCOVERY_DONE:
    return g_strdup_printf(tpl, "done");
    break;
  case ARTIK_ZIGBEE_DEVICE_DISCOVERY_NO_DEVICE:
    return g_strdup_printf(tpl, "no_device");
    break;
  case ARTIK_ZIGBEE_DEVICE_DISCOVERY_ERROR:
    return g_strdup_printf(tpl, "error");
    break;
  case ARTIK_ZIGBEE_DEVICE_DISCOVERY_CHANGED:
    return g_strdup_printf(tpl_dev, "changed",
        _convert_device(&(device_discovery->device)));
    break;
  case ARTIK_ZIGBEE_DEVICE_DISCOVERY_LOST:
    return g_strdup_printf(tpl_dev, "lost",
        _convert_device(&(device_discovery->device)));
    break;
  default:
    log_err("unknown status(%d)", device_discovery->status);
    break;
  }

  return g_strdup_printf(tpl, "unknown");
}

static char* _convert_ieee_addr_resp(const void *payload) {
  const char *tpl_fail = "{ "
      "\"type\": \"ieee_addr\", "
      "\"status\": \"%s\""
      " }";
  const char *tpl = "{ "
      "\"type\": \"ieee_addr\", "
      "\"status\": \"%s\", "
      "\"node_id\": %d, "
      "\"eui64\": \"%s\""
      " }";
  char *eui64;
  char *json_res;

  const artik_zigbee_ieee_addr_response *addr_rsp =
      reinterpret_cast<const artik_zigbee_ieee_addr_response *>(payload);

  switch (addr_rsp->result) {
  case ARTIK_ZIGBEE_SERVICE_DISCOVERY_DONE:
    eui64 = _convert_eui64(addr_rsp->eui64);
    json_res = g_strdup_printf(tpl, "success", addr_rsp->node_id, eui64);
    g_free(eui64);
    return json_res;
    break;
  case ARTIK_ZIGBEE_SERVICE_DISCOVERY_ERROR:
    break;
  default:
    log_err("unknown result(%d)", addr_rsp->result);
    break;
  }

  return g_strdup_printf(tpl_fail, "error");
}

static char* _convert_simple_desc_resp(const void *payload) {
  const char *tpl_fail = "{ "
      "\"type\": \"simple_desc\", "
      "\"status\": \"%s\""
      " }";
  const char *tpl = "{ "
      "\"type\": \"simple_desc\", "
      "\"status\": \"%s\", "
      "\"target_node_id\": %d, "
      "\"target_endpoint\": %d, "
      "\"server_clusters\": %s, "
      "\"client_clusters\": %s "
      " }";
  const artik_zigbee_simple_descriptor_response *simple_descriptor =
      reinterpret_cast<const artik_zigbee_simple_descriptor_response*>(payload);
  gchar *server_clusters;
  gchar *client_clusters;
  gchar *result;

  switch (simple_descriptor->result) {
  case ARTIK_ZIGBEE_SERVICE_DISCOVERY_DONE:
    server_clusters = _convert_int_list(
        simple_descriptor->server_cluster_count,
        simple_descriptor->server_cluster);
    client_clusters = _convert_int_list(
        simple_descriptor->client_cluster_count,
        simple_descriptor->client_cluster);

    result = g_strdup_printf(tpl, "success",
        simple_descriptor->target_node_id,
        simple_descriptor->target_endpoint, server_clusters,
        client_clusters);

    g_free(server_clusters);
    g_free(client_clusters);

    return result;
    break;
  case ARTIK_ZIGBEE_SERVICE_DISCOVERY_ERROR:
    break;
  default:
    log_err("unknown result(%d)", simple_descriptor->result);
    break;
  }

  return g_strdup_printf(tpl_fail, "error");
}

static char* _convert_match_desc_resp(const void *payload) {
  const char *tpl_simple = "{ "
      "\"type\": \"match_desc\", "
      "\"status\": \"%s\""
      " }";
  const char *tpl = "{ "
      "\"type\": \"match_desc\", "
      "\"status\": \"%s\", "
      "\"node_id\": %d, "
      "\"endpoints\": %s "
      " }";
  const artik_zigbee_match_desc_response *match_desc =
      reinterpret_cast<const artik_zigbee_match_desc_response *>(payload);
  gchar *endpoints;
  gchar *result;

  switch (match_desc->result) {
  case ARTIK_ZIGBEE_SERVICE_DISCOVERY_DONE:
    return g_strdup_printf(tpl_simple, "success");
    break;
  case ARTIK_ZIGBEE_SERVICE_DISCOVERY_RECEIVED:
    endpoints = _convert_int_list(match_desc->count,
        match_desc->endpoint_list);

    result = g_strdup_printf(tpl, "received", match_desc->node_id,
        endpoints);

    g_free(endpoints);
    return result;
    break;
  case ARTIK_ZIGBEE_SERVICE_DISCOVERY_ERROR:
    break;
  default:
    log_err("unknown result(%d)", match_desc->result);
    break;
  }

  return g_strdup_printf(tpl_simple, "error");
}

static char* _convert_network_find(const void *payload) {
  const char *tpl_fail = "{ "
      "\"type\": \"network_find\", "
      "\"status\": \"%s\""
      " }";
  const char *tpl = "{ "
      "\"type\": \"network_find\", "
      "\"status\": \"%s\", "
      "\"channel\": %d, "
      "\"tx_power\": %d, "
      "\"pan_id\": %d "
      " }";
  const artik_zigbee_network_find_result *net_find =
      reinterpret_cast<const artik_zigbee_network_find_result*>(payload);

  switch (net_find->find_status) {
  case ARTIK_ZIGBEE_NETWORK_FOUND:
    return g_strdup_printf(tpl, "found", net_find->network_info.channel,
        net_find->network_info.tx_power, net_find->network_info.pan_id);
    break;
  case ARTIK_ZIGBEE_NETWORK_FIND_FINISHED:
    return g_strdup_printf(tpl_fail, "finished");
    break;
  case ARTIK_ZIGBEE_NETWORK_FIND_ERR:
    break;
  default:
    log_err("unknown status(%d)", net_find->find_status);
    break;
  }

  return g_strdup_printf(tpl_fail, "error");
}

static char* _convert_groups_info(const void *payload) {
  const char *tpl = "{ "
      "\"type\": \"groups_info\", "
      "\"command\": \"%s\", "
      "\"group_id\": %d, "
      "\"endpoint_id\": %d"
      " }";
  const artik_zigbee_groups_info *group_info =
      reinterpret_cast<const artik_zigbee_groups_info *>(payload);

  switch (group_info->group_cmd) {
  case ARTIK_ZIGBEE_GROUPS_ADD_IF_IDENTIFYING:
    g_strdup_printf(tpl, "add_if_identifying", group_info->group_id,
        group_info->endpoint_id);
    break;
  case ARTIK_ZIGBEE_GROUPS_ADD:
    g_strdup_printf(tpl, "add", group_info->group_id,
        group_info->endpoint_id);
    break;
  case ARTIK_ZIGBEE_GROUPS_REMOVE:
    g_strdup_printf(tpl, "remove", group_info->group_id,
        group_info->endpoint_id);
    break;
  case ARTIK_ZIGBEE_GROUPS_REMOVE_ALL:
    g_strdup_printf(tpl, "remove_all", group_info->group_id,
        group_info->endpoint_id);
    break;
  default:
    log_err("unknown cmd(%d)", group_info->group_cmd);
    break;
  }

  return g_strdup_printf(tpl, "error", 0, 0);
}

static char* _convert_attribute_change(const void *payload) {
  const char *tpl = "{ "
      "\"type\": \"attribute_change\", "
      "\"attr\": \"%s\", "
      "\"endpoint_id\": %d"
      " }";
  const artik_zigbee_attribute_changed_response *attr_info =
    reinterpret_cast<const artik_zigbee_attribute_changed_response *>(payload);

  switch (attr_info->type) {
  case ARTIK_ZIGBEE_ATTR_ONOFF_STATUS:
    return g_strdup_printf(tpl, "onoff_status", attr_info->endpoint_id);
    break;
  case ARTIK_ZIGBEE_ATTR_LEVELCONTROL_LEVEL:
    return g_strdup_printf(tpl, "levelcontrol_level",
        attr_info->endpoint_id);
    break;
  case ARTIK_ZIGBEE_ATTR_COLOR_HUE:
    return g_strdup_printf(tpl, "color_hue", attr_info->endpoint_id);
    break;
  case ARTIK_ZIGBEE_ATTR_COLOR_SATURATION:
    return g_strdup_printf(tpl, "color_saturation", attr_info->endpoint_id);
    break;
  case ARTIK_ZIGBEE_ATTR_COLOR_CURRENT_X:
    return g_strdup_printf(tpl, "color_current_x", attr_info->endpoint_id);
    break;
  case ARTIK_ZIGBEE_ATTR_COLOR_CURRENT_Y:
    return g_strdup_printf(tpl, "color_current_y", attr_info->endpoint_id);
    break;
  case ARTIK_ZIGBEE_ATTR_COLOR_TEMP:
    return g_strdup_printf(tpl, "color_temp", attr_info->endpoint_id);
    break;
  case ARTIK_ZIGBEE_ATTR_FAN_MODE:
    return g_strdup_printf(tpl, "fan_mode", attr_info->endpoint_id);
    break;
  case ARTIK_ZIGBEE_ATTR_FAN_MODE_SEQUENCE:
    return g_strdup_printf(tpl, "fan_mode_sequence", attr_info->endpoint_id);
    break;
  case ARTIK_ZIGBEE_ATTR_OCCUPIED_HEATING_SETPOINT:
    return g_strdup_printf(tpl, "occupied_heating_setpoint",
        attr_info->endpoint_id);
    break;
  case ARTIK_ZIGBEE_ATTR_OCCUPIED_COOLING_SETPOINT:
    return g_strdup_printf(tpl, "occupied_cooling_setpoint",
        attr_info->endpoint_id);
    break;
  case ARTIK_ZIGBEE_ATTR_SYSTEM_MODE:
    return g_strdup_printf(tpl, "system_mode", attr_info->endpoint_id);
    break;
  case ARTIK_ZIGBEE_ATTR_CONTROL_SEQUENCE:
    return g_strdup_printf(tpl, "control_sequence", attr_info->endpoint_id);
    break;
  case ARTIK_ZIGBEE_ATTR_ILLUMINANCE:
    return g_strdup_printf(tpl, "illuminance", attr_info->endpoint_id);
    break;
  case ARTIK_ZIGBEE_ATTR_TEMPERATURE:
    return g_strdup_printf(tpl, "temperature", attr_info->endpoint_id);
    break;
  case ARTIK_ZIGBEE_ATTR_OCCUPANCY:
    return g_strdup_printf(tpl, "occupancy", attr_info->endpoint_id);
    break;
  case ARTIK_ZIGBEE_ATTR_THERMOSTAT_TEMPERATURE:
    return g_strdup_printf(tpl, "thermostat_temperature",
        attr_info->endpoint_id);
    break;
  case ARTIK_ZIGBEE_ATTR_NONE:
    return g_strdup_printf(tpl, "none", attr_info->endpoint_id);
    break;
  default:
    log_err("unknown attribute type(%d)", attr_info->type);
    break;
  }

  return g_strdup_printf(tpl, "error");
}

static char* _convert_receive_command(const void *payload) {
  const char *tpl = "{ "
      "\"type\": \"receive_command\", "
      "\"is_global_command\": %d, "
      "\"endpoint_id\": %d, "
      "\"cluster_id\": %d, "
      "\"command_id\": %d, "
      "\"payload\": %s, "
      "\"source_device_id\": %d, "
      "\"source_endpoint_id\": %d "
      " }";
  const artik_zigbee_received_command *received_command =
      reinterpret_cast<const artik_zigbee_received_command *>(payload);
  gchar *cmd_payload;
  gchar *result;

  cmd_payload = _convert_char_list(received_command->payload_length,
      received_command->payload);

  result = g_strdup_printf(tpl, received_command->is_global_command,
      received_command->dest_endpoint_id, received_command->cluster_id,
      received_command->command_id, cmd_payload,
      received_command->source_node_id,
      received_command->source_endpoint_id);
  g_free(cmd_payload);

  return result;
}

static char* _convert_reporting_configure(const void *payload) {
  const char *tpl = "{ "
      "\"type\": \"reporting_configure\", "
      "\"used\": %d, "
      "\"endpoint_id\": %d, "
      "\"cluster_id\": %d, "
      "\"attribute_id\": %d, "
      "\"is_server\": %d, "
      "\"reported\": { "
      "  \"min_interval\": %d, "
      "  \"max_interval\": %d, "
      "  \"reportable_change\": %d "
      " }"
      " }";
  const artik_zigbee_reporting_info *reporting_info =
      reinterpret_cast<const artik_zigbee_reporting_info *>(payload);

  return g_strdup_printf(tpl, reporting_info->used,
      reporting_info->endpoint_id, reporting_info->cluster_id,
      reporting_info->attribute_id, reporting_info->is_server,
      reporting_info->reported.min_interval,
      reporting_info->reported.max_interval,
      reporting_info->reported.reportable_change);
}

static char* _convert_report_attribute(const void *payload) {
  const char *tpl = "{ "
      "\"type\": \"report_attribute\", "
      "\"attr\": \"%s\", "
      "\"value\": %d"
      " }";
  const artik_zigbee_report_attribute_info *report_attr_info =
      reinterpret_cast<const artik_zigbee_report_attribute_info *>(payload);

  switch (report_attr_info->attribute_type) {
  case ARTIK_ZIGBEE_ATTR_ILLUMINANCE:
    return g_strdup_printf(tpl, "illuminance",
        report_attr_info->data.value);
    break;
  case ARTIK_ZIGBEE_ATTR_TEMPERATURE:
    return g_strdup_printf(tpl, "temperature",
        report_attr_info->data.value);
    break;
  case ARTIK_ZIGBEE_ATTR_OCCUPANCY:
    return g_strdup_printf(tpl, "occupancy",
        report_attr_info->data.occupancy);
    break;
  case ARTIK_ZIGBEE_ATTR_THERMOSTAT_TEMPERATURE:
    return g_strdup_printf(tpl, "thermostat_temperature",
        report_attr_info->data.value);
    break;
  case ARTIK_ZIGBEE_ATTR_NONE:
    return g_strdup_printf(tpl, "none", 0);
    break;
  default:
    log_err("unknown attribute type(%d)", report_attr_info->attribute_type);
    break;
  }

  return g_strdup_printf(tpl, "unknown", 0);
}

static char* _convert_identify_feedback_start(const void *payload) {
  const char *tpl = "{ "
      "\"type\": \"identify_feedback_start\", "
      "\"endpoint_id\": %d, "
      "\"duration\": %d"
      " }";
  const artik_zigbee_identify_feedback_info *identify_info =
      reinterpret_cast<const artik_zigbee_identify_feedback_info *>(payload);

  return g_strdup_printf(tpl, identify_info->endpoint_id,
      identify_info->duration);
}

static char* _convert_identify_feedback_stop(const void *payload) {
  const char *tpl = "{ "
      "\"type\": \"identify_feedback_stop\", "
      "\"endpoint_id\": %d, "
      "\"duration\": %d"
      " }";
  const artik_zigbee_identify_feedback_info *identify_info =
      reinterpret_cast<const artik_zigbee_identify_feedback_info *>(payload);

  return g_strdup_printf(tpl, identify_info->endpoint_id,
      identify_info->duration);
}

static char* _convert_commissioning_status(const void *payload) {
  const char *tpl = "{ "
      "\"type\": \"commissioning_status\", "
      "\"status\": \"%s\" "
      " }";
  const artik_zigbee_commissioning_state commissioning_state =
      *(reinterpret_cast<const artik_zigbee_commissioning_state *>(payload));

  switch (commissioning_state) {
  case ARTIK_ZIGBEE_COMMISSIONING_ERR_IN_PROGRESS:
    return g_strdup_printf(tpl, "error_in_progress");
    break;
  case ARTIK_ZIGBEE_COMMISSIONING_NETWORK_STEERING_FORM:
    return g_strdup_printf(tpl, "network_steering_form");
    break;
  case ARTIK_ZIGBEE_COMMISSIONING_NETWORK_STEERING_SUCCESS:
    return g_strdup_printf(tpl, "network_steering_success");
    break;
  case ARTIK_ZIGBEE_COMMISSIONING_NETWORK_STEERING_FAILED:
    return g_strdup_printf(tpl, "network_steering_failed");
    break;
  case ARTIK_ZIGBEE_COMMISSIONING_WAIT_NETWORK_STEERING:
    return g_strdup_printf(tpl, "network_steering");
    break;
  case ARTIK_ZIGBEE_COMMISSIONING_INITIATOR_SUCCESS:
    return g_strdup_printf(tpl, "initiator_success");
    break;
  case ARTIK_ZIGBEE_COMMISSIONING_INITIATOR_FAILED:
    return g_strdup_printf(tpl, "initiator_failed");
    break;
  case ARTIK_ZIGBEE_COMMISSIONING_INITIATOR_STOP:
    return g_strdup_printf(tpl, "initiator_stop");
    break;
  case ARTIK_ZIGBEE_COMMISSIONING_TARGET_SUCCESS:
    return g_strdup_printf(tpl, "target_success");
    break;
  case ARTIK_ZIGBEE_COMMISSIONING_TARGET_FAILED:
    return g_strdup_printf(tpl, "target_failed");
    break;
  case ARTIK_ZIGBEE_COMMISSIONING_TARGET_STOP:
    return g_strdup_printf(tpl, "target_stop");
    break;
  default:
    log_err("unknown state(%d)", commissioning_state);
    break;
  }

  return g_strdup_printf(tpl, "error");
}

static char* _convert_commissioning_target_info(const void *payload) {
  const char *tpl = "{ "
      "\"type\": \"commissioning_target_info\", "
      "\"node_id\": %d, "
      "\"endpoint_id\": %d"
      " }";
  const artik_zigbee_commissioning_target_info *target_info =
      reinterpret_cast<const artik_zigbee_commissioning_target_info*>(payload);

  return g_strdup_printf(tpl, target_info->node_id,
      target_info->endpoint_id);
}

static char* _convert_commissioning_bound_info(const void *payload) {
  const char *tpl = "{ "
      "\"type\": \"commissioning_bound_info\", "
      "\"node_id\": %d, "
      "\"cluster_id\": %d, "
      "\"endpoint_id\": %d"
      " }";
  const artik_zigbee_commissioning_bound_info *bound_info =
      reinterpret_cast<const artik_zigbee_commissioning_bound_info *>(payload);

  return g_strdup_printf(tpl, bound_info->node_id, bound_info->cluster_id,
      bound_info->endpoint_id);
}

static char* _convert_basic_reset_to_factory(const void *payload) {
  const char *tpl = "{ "
      "\"type\": \"basic_reset_to_factory\", "
      "\"endpoint_id\": %d"
      " }";

  return g_strdup_printf(tpl, *(reinterpret_cast<const int *>(payload)));
}

static char* _convert_broadcast_identify_query_response(const void *payload) {
  const char *tpl = "{ "
      "\"type\": \"broadcast_identify_query_response\", "
      "\"node_id\": %d, "
      "\"endpoint_id\": %d, "
      "\"timeout\": %d"
      " }";

  const artik_zigbee_broadcast_identify_query_response *resp =
     reinterpret_cast<const artik_zigbee_broadcast_identify_query_response *>(
         payload);

  return g_strdup_printf(tpl, resp->node_id, resp->endpoint_id,
      resp->timeout);
}

static char* _convert_level_control(const void *payload) {
  const char *tpl = "{ "
      "\"type\": \"level_control\", "
      "\"control_type\": \"%s\","
      "\"value\": %d,"
      "\"transition_time\": %d,"
      "\"onoff\": %s"
      " }";
  const char *tpl_move = "{ "
      "\"type\": \"level_control\", "
      "\"control_type\": \"%s\","
      "\"value\": %d,"
      "\"onoff\": %s"
      " }";
  const char *tpl_stop = "{ "
      "\"type\": \"level_control\", "
      "\"control_type\": \"stop\","
      "\"onoff\": %s"
      " }";
  const char *tpl_error = "{ "
      "\"type\": \"level_control\", "
      "\"control_type\": \"error\""
      " }";
  const artik_zigbee_level_control_command *cmd =
      reinterpret_cast<const artik_zigbee_level_control_command *>(payload);

  switch (cmd->control_type) {
  case ARTIK_ZIGBEE_MOVE_TO_LEVEL:
  case ARTIK_ZIGBEE_MOVE_TO_LEVEL_ONOFF:
    return g_strdup_printf(tpl, "moveto",
        cmd->parameters.move_to_level.level,
        cmd->parameters.move_to_level.transition_time,
        (cmd->control_type == ARTIK_ZIGBEE_MOVE_TO_LEVEL_ONOFF) ?
            "true" : "false");
    break;
  case ARTIK_ZIGBEE_MOVE:
  case ARTIK_ZIGBEE_MOVE_ONOFF:
    if (cmd->parameters.move.control_mode == ARTIK_ZIGBEE_LEVEL_CONTROL_UP)
      return g_strdup_printf(tpl_move, "moveup",
          cmd->parameters.move.rate,
          (cmd->control_type == ARTIK_ZIGBEE_MOVE_ONOFF) ? "true" : "false");
    else if (cmd->parameters.move.control_mode ==
                ARTIK_ZIGBEE_LEVEL_CONTROL_DOWN)
      return g_strdup_printf(tpl_move, "movedown",
          cmd->parameters.move.rate,
          (cmd->control_type == ARTIK_ZIGBEE_MOVE_ONOFF) ? "true" : "false");
    break;
  case ARTIK_ZIGBEE_STEP:
  case ARTIK_ZIGBEE_STEP_ONOFF:
    if (cmd->parameters.step.control_mode == ARTIK_ZIGBEE_LEVEL_CONTROL_UP)
      return g_strdup_printf(tpl, "stepup",
          cmd->parameters.step.step_size,
          cmd->parameters.step.transition_time,
          (cmd->control_type == ARTIK_ZIGBEE_STEP_ONOFF) ? "true" : "false");
    else if (cmd->parameters.step.control_mode ==
                ARTIK_ZIGBEE_LEVEL_CONTROL_DOWN)
      return g_strdup_printf(tpl, "stepdown",
          cmd->parameters.step.step_size,
          cmd->parameters.step.transition_time,
          (cmd->control_type == ARTIK_ZIGBEE_STEP_ONOFF) ? "true" : "false");
    break;
  case ARTIK_ZIGBEE_STOP:
  case ARTIK_ZIGBEE_STOP_ONOFF:
    return g_strdup_printf(tpl_stop,
        (cmd->control_type == ARTIK_ZIGBEE_STOP_ONOFF) ? "true" : "false");
    break;
  default:
    break;
  }

  return g_strdup(tpl_error);
}

static char* _convert_unknown(artik_zigbee_response_type response_type) {
  const char *tpl = "{ "
      "\"type\": \"unknown\", "
      "\"type_id\": %d"
      " }";

  return g_strdup_printf(tpl, response_type);
}

int convert_jsobject_levelcontrol(Isolate* isolate, const Local<Object>& in,
    artik_zigbee_level_control_command *out) {

  v8::String::Utf8Value str_type(
      in->Get(String::NewFromUtf8(isolate, "type"))->ToString());
  char *type = *str_type;
  bool onoff = false;
  int transition_time = 0;

  memset(out, 0, sizeof(artik_zigbee_level_control_command));

  Local<Value> js_auto_onoff = in->Get(
      String::NewFromUtf8(isolate, "auto_onoff"));
  if (js_auto_onoff->IsBoolean())
    onoff = js_auto_onoff->BooleanValue();

  if (!g_strcmp0(type, "stop")) {
    if (onoff)
      out->control_type = ARTIK_ZIGBEE_STOP_ONOFF;
    else
      out->control_type = ARTIK_ZIGBEE_STOP;
    return 0;
  }

  Local<Value> js_value = in->Get(
      String::NewFromUtf8(isolate, "value"));
  if (!js_value->IsInt32()) {
    log_err("missing 'value' field");
    return -1;
  }

  Local<Value> js_transition_time = in->Get(
      String::NewFromUtf8(isolate, "transition_time"));
  if (js_transition_time->IsInt32())
    transition_time = js_transition_time->Int32Value();

  log_dbg("type:'%s', onoff: %d, time: %d", type, onoff, transition_time);

  if (!g_strcmp0(type, "moveup")) {
    if (onoff)
      out->control_type = ARTIK_ZIGBEE_MOVE_ONOFF;
    else
      out->control_type = ARTIK_ZIGBEE_MOVE;
    out->parameters.move.control_mode = ARTIK_ZIGBEE_LEVEL_CONTROL_UP;
    out->parameters.move.rate = js_value->Int32Value();
  } else if (!g_strcmp0(type, "movedown")) {
    if (onoff)
      out->control_type = ARTIK_ZIGBEE_MOVE_ONOFF;
    else
      out->control_type = ARTIK_ZIGBEE_MOVE;
    out->parameters.move.control_mode = ARTIK_ZIGBEE_LEVEL_CONTROL_DOWN;
    out->parameters.move.rate = js_value->Int32Value();
  } else if (!g_strcmp0(type, "moveto")) {
    if (onoff)
      out->control_type = ARTIK_ZIGBEE_MOVE_TO_LEVEL_ONOFF;
    else
      out->control_type = ARTIK_ZIGBEE_MOVE_TO_LEVEL;
    out->parameters.move_to_level.level = js_value->Int32Value();
    out->parameters.move_to_level.transition_time = transition_time;
  } else if (!g_strcmp0(type, "stepup")) {
    if (onoff)
      out->control_type = ARTIK_ZIGBEE_STEP_ONOFF;
    else
      out->control_type = ARTIK_ZIGBEE_STEP;
    out->parameters.step.control_mode = ARTIK_ZIGBEE_LEVEL_CONTROL_UP;
    out->parameters.step.step_size = js_value->Int32Value();
    out->parameters.step.transition_time = transition_time;
  } else if (!g_strcmp0(type, "stepdown")) {
    if (onoff)
      out->control_type = ARTIK_ZIGBEE_STEP_ONOFF;
    else
      out->control_type = ARTIK_ZIGBEE_STEP;
    out->parameters.step.control_mode = ARTIK_ZIGBEE_LEVEL_CONTROL_DOWN;
    out->parameters.step.step_size = js_value->Int32Value();
    out->parameters.step.transition_time = transition_time;
  } else {
    log_err("unknown type(%s)", type);
    return -1;
  }

  return 0;
}

int convert_jsobject_endpoint(Isolate* isolate, const Local<Object>& in,
    artik_zigbee_endpoint *out) {

  Local<Value> js_node_id = in->Get(
      String::NewFromUtf8(isolate, "node_id"));
  Local<Value> js_endpoint_id = in->Get(
      String::NewFromUtf8(isolate, "endpoint_id"));
  Local<Array> server_cluster = Local<Array>::Cast(in->Get(
      String::NewFromUtf8(isolate, "server_cluster")));
  Local<Array> client_cluster = Local<Array>::Cast(in->Get(
      String::NewFromUtf8(isolate, "client_cluster")));
  int i;

  out->node_id = js_node_id->Int32Value();
  out->endpoint_id = js_endpoint_id->Int32Value();

  for (i = 0; i < ARTIK_ZIGBEE_MAX_CLUSTER_SIZE; i++) {
    out->server_cluster[i] = server_cluster->Get(i)->Int32Value();
    out->client_cluster[i] = client_cluster->Get(i)->Int32Value();
  }

  return 0;
}

char *convert_device_info(const artik_zigbee_device_info *di) {
  const artik_zigbee_device *dev;
  gchar **str_devices;
  char *tmp, *json;

  str_devices = g_new0(gchar *, di->num + 1);

  log_dbg("- device num: %d", di->num);

  for (int i = 0; i < di->num; i++) {
    dev = reinterpret_cast<const artik_zigbee_device *>(&(di->device[i]));
    str_devices[i] = _convert_device(dev);
    log_dbg("- [%d] %s", i, str_devices[i]);
  }

  tmp = g_strjoinv(",", str_devices);
  g_strfreev(str_devices);

  json = g_strdup_printf("[%s]", tmp);
  g_free(tmp);

  return json;
}

char* convert_endpointlist_to_json(const artik_zigbee_endpoint_list *list) {
  return _convert_endpointlist_to_json_full(list->num, list->endpoint);
}

void zb_callback(void *user_data, artik_zigbee_response_type response_type,
                 void *payload) {
  Isolate * isolate = Isolate::GetCurrent();
  v8::HandleScope handleScope(isolate);
  ZigbeeWrapper* wrap = reinterpret_cast<ZigbeeWrapper*>(user_data);
  converter_func func = NULL;
  char *json_res;

  log_dbg("on_callback - response_type: %d", response_type);

  switch (response_type) {
  case ARTIK_ZIGBEE_RESPONSE_NOTIFICATION:
    func = _convert_notification;
    break;
  case ARTIK_ZIGBEE_RESPONSE_CLIENT_TO_SERVER_COMMAND_RECEIVED:
    func = _convert_receive_command;
    break;
  case ARTIK_ZIGBEE_RESPONSE_ATTRIBUTE_CHANGE:
    func = _convert_attribute_change;
    break;
  case ARTIK_ZIGBEE_RESPONSE_REPORTING_CONFIGURE:
    func = _convert_reporting_configure;
    break;
  case ARTIK_ZIGBEE_RESPONSE_REPORT_ATTRIBUTE:
    func = _convert_report_attribute;
    break;
  case ARTIK_ZIGBEE_RESPONSE_IDENTIFY_FEEDBACK_START:
    func = _convert_identify_feedback_start;
    break;
  case ARTIK_ZIGBEE_RESPONSE_IDENTIFY_FEEDBACK_STOP:
    func = _convert_identify_feedback_stop;
    break;

  /* Network response */
  case ARTIK_ZIGBEE_RESPONSE_NETWORK_NOTIFICATION:
    func = _convert_network_notification;
    break;
  case ARTIK_ZIGBEE_RESPONSE_NETWORK_FIND:
    func = _convert_network_find;
    break;

  /* Device response */
  case ARTIK_ZIGBEE_RESPONSE_DEVICE_DISCOVER:
    func = _convert_device_discover;
    break;

  /* Cluster response */
  case ARTIK_ZIGBEE_RESPONSE_BROADCAST_IDENTIFY_QUERY:
    func = _convert_broadcast_identify_query_response;
    break;
  case ARTIK_ZIGBEE_RESPONSE_GROUPS_INFO:
    func = _convert_groups_info;
    break;
  case ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_STATUS:
    func = _convert_commissioning_status;
    break;
  case ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_TARGET_INFO:
    func = _convert_commissioning_target_info;
    break;
  case ARTIK_ZIGBEE_RESPONSE_COMMISSIONING_BOUND_INFO:
    func = _convert_commissioning_bound_info;
    break;
  case ARTIK_ZIGBEE_RESPONSE_IEEE_ADDR_RESP:
    func = _convert_ieee_addr_resp;
    break;
  case ARTIK_ZIGBEE_RESPONSE_SIMPLE_DESC_RESP:
    func = _convert_simple_desc_resp;
    break;
  case ARTIK_ZIGBEE_RESPONSE_MATCH_DESC_RESP:
    func = _convert_match_desc_resp;
    break;
  case ARTIK_ZIGBEE_RESPONSE_BASIC_RESET_TO_FACTORY:
    func = _convert_basic_reset_to_factory;
    break;
  case ARTIK_ZIGBEE_RESPONSE_LEVEL_CONTROL:
    func = _convert_level_control;
    break;
  case ARTIK_ZIGBEE_RESPONSE_NONE:
    break;
  default:
    log_err("unknown event(%d)", response_type);
    break;
  }

  if (func)
    json_res = func(payload);
  else
    json_res = _convert_unknown(response_type);

  Handle<Value> argv[] =
      { Handle<Value>(String::NewFromUtf8(isolate, json_res)) };

  if (json_res)
    free(json_res);

  Local<Function>::New(isolate, *wrap->getIintCb())->Call(
      isolate->GetCurrentContext()->Global(), 1, argv);
}

void throw_error(Isolate *isolate, artik_error code) {
  const char *tpl = "{ "
      "\"code\": %d, "
      "\"message\": \"%s\""
      " }";
  gchar *json_res;

  log_dbg("- return: %d (0x%X)", code, code);

  switch (code) {
  case E_BAD_ARGS:
    json_res = g_strdup_printf(tpl, code, "E_BAD_ARGS");
    break;
  case E_BUSY:
    json_res = g_strdup_printf(tpl, code, "E_BUSY");
    break;
  case E_NOT_INITIALIZED:
    json_res = g_strdup_printf(tpl, code, "E_NOT_INITIALIZED");
    break;
  case E_NO_MEM:
    json_res = g_strdup_printf(tpl, code, "E_NO_MEM");
    break;
  case E_NOT_SUPPORTED:
    json_res = g_strdup_printf(tpl, code, "E_NOT_SUPPORTED");
    break;
  case E_OVERFLOW:
    json_res = g_strdup_printf(tpl, code, "E_OVERFLOW");
    break;
  case E_ACCESS_DENIED:
    json_res = g_strdup_printf(tpl, code, "E_ACCESS_DENIED");
    break;
  case E_INTERRUPTED:
    json_res = g_strdup_printf(tpl, code, "E_INTERRUPTED");
    break;
  case E_TRY_AGAIN:
    json_res = g_strdup_printf(tpl, code, "E_TRY_AGAIN");
    break;
  case E_TIMEOUT:
    json_res = g_strdup_printf(tpl, code, "E_TIMEOUT");
    break;
  case E_INVALID_VALUE:
    json_res = g_strdup_printf(tpl, code, "E_INVALID_VALUE");
    break;
  case E_ZIGBEE_ERROR:
    json_res = g_strdup_printf(tpl, code, "E_ZIGBEE_ERROR");
    break;
  case E_ZIGBEE_INVALID_DAEMON:
    json_res = g_strdup_printf(tpl, code, "E_ZIGBEE_INVALID_DAEMON");
    break;
  case E_ZIGBEE_NO_DAEMON:
    json_res = g_strdup_printf(tpl, code, "E_ZIGBEE_NO_DAEMON");
    break;
  case E_ZIGBEE_NO_MESSAGE:
    json_res = g_strdup_printf(tpl, code, "E_ZIGBEE_NO_MESSAGE");
    break;
  case E_ZIGBEE_NO_DEVICE:
    json_res = g_strdup_printf(tpl, code, "E_ZIGBEE_NO_DEVICE");
    break;
  case E_ZIGBEE_ERR_SOCK:
    json_res = g_strdup_printf(tpl, code, "E_ZIGBEE_ERR_SOCK");
    break;
  case E_ZIGBEE_MSG_SEND_ERROR:
    json_res = g_strdup_printf(tpl, code, "E_ZIGBEE_MSG_SEND_ERROR");
    break;
  case E_ZIGBEE_NETWORK_EXIST:
    json_res = g_strdup_printf(tpl, code, "E_ZIGBEE_NETWORK_EXIST");
    break;
  default:
    json_res = g_strdup_printf(tpl, code, "Unknown Error");
    break;
  }

  isolate->ThrowException(
      Exception::Error(String::NewFromUtf8(isolate, json_res)));

  g_free(json_res);
}

}  // namespace artik
