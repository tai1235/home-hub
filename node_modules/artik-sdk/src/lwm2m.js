var events = require('events');
var util = require('util');
var lwm2m = require('../build/Release/artik-sdk.node').lwm2m;

var Lwm2m = function(){
    events.EventEmitter.call(this);
    this.lwm2m = new lwm2m();
    setImmediate(function(self) {
        self.emit('started');
    }, this);
}

util.inherits(Lwm2m, events.EventEmitter);

module.exports = Lwm2m;

Lwm2m.prototype.LWM2M_URI_DEVICE_MANUFACTURER = "/3/0/0";
Lwm2m.prototype.LWM2M_URI_DEVICE_MODEL_NUM = "/3/0/1";
Lwm2m.prototype.LWM2M_URI_DEVICE_SERIAL_NUM = "/3/0/2";
Lwm2m.prototype.LWM2M_URI_DEVICE_FW_VERSION = "/3/0/3";
Lwm2m.prototype.LWM2M_URI_DEVICE_REBOOT = "/3/0/4";
Lwm2m.prototype.LWM2M_URI_DEVICE_FACTORY_RESET = "/3/0/5";
Lwm2m.prototype.LWM2M_URI_DEVICE_POWER_SOURCES = "/3/0/6";
Lwm2m.prototype.LWM2M_URI_DEVICE_POWER_VOLTAGE = "/3/0/7";
Lwm2m.prototype.LWM2M_URI_DEVICE_POWER_CURRENT = "/3/0/8";
Lwm2m.prototype.LWM2M_URI_DEVICE_BATT_LEVEL = "/3/0/9";
Lwm2m.prototype.LWM2M_URI_DEVICE_MEMORY_FREE = "/3/0/10";
Lwm2m.prototype.LWM2M_URI_DEVICE_ERROR_CODE = "/3/0/11";
Lwm2m.prototype.LWM2M_URI_DEVICE_RESET_ERR_CODE = "/3/0/12";
Lwm2m.prototype.LWM2M_URI_DEVICE_CURRENT_TIME = "/3/0/13";
Lwm2m.prototype.LWM2M_URI_DEVICE_UTC_OFFSET = "/3/0/14";
Lwm2m.prototype.LWM2M_URI_DEVICE_TIMEZONE = "/3/0/15";
Lwm2m.prototype.LWM2M_URI_DEVICE_SUPP_BIND_MODES = "/3/0/16";
Lwm2m.prototype.LWM2M_URI_DEVICE_DEVICE_TYPE = "/3/0/17";
Lwm2m.prototype.LWM2M_URI_DEVICE_HW_VERSION = "/3/0/18";
Lwm2m.prototype.LWM2M_URI_DEVICE_SW_VERSION = "/3/0/19";
Lwm2m.prototype.LWM2M_URI_DEVICE_BATT_STATUS = "/3/0/20";
Lwm2m.prototype.LWM2M_URI_DEVICE_MEMORY_TOTAL = "/3/0/21";
Lwm2m.prototype.LWM2M_URI_CONNMON_BEARER = "/4/0/0";
Lwm2m.prototype.LWM2M_URI_CONNMON_AVAIL_BEARERS = "/4/0/1";
Lwm2m.prototype.LWM2M_URI_CONNMON_SIGNAL = "/4/0/2";
Lwm2m.prototype.LWM2M_URI_CONNMON_LINK_QUALITY = "/4/0/3";
Lwm2m.prototype.LWM2M_URI_CONNMON_IP_ADDR = "/4/0/4";
Lwm2m.prototype.LWM2M_URI_CONNMON_ROUTER_IP_ADDR = "/4/0/5";
Lwm2m.prototype.LWM2M_URI_CONNMON_LINK_UTIL = "/4/0/6";
Lwm2m.prototype.LWM2M_URI_CONNMON_APN = "/4/0/7";
Lwm2m.prototype.LWM2M_URI_CONNMON_CELL_ID = "/4/0/8";
Lwm2m.prototype.LWM2M_URI_CONNMON_SMNC = "/4/0/9";
Lwm2m.prototype.LWM2M_URI_CONNMON_SMCC = "/4/0/10";
Lwm2m.prototype.LWM2M_URI_FIRMWARE_PACKAGE = "/5/0/0";
Lwm2m.prototype.LWM2M_URI_FIRMWARE_PACKAGE_URI = "/5/0/1";
Lwm2m.prototype.LWM2M_URI_FIRMWARE_UPDATE = "/5/0/2";
Lwm2m.prototype.LWM2M_URI_FIRMWARE_STATE = "/5/0/3";
Lwm2m.prototype.LWM2M_URI_FIRMWARE_UPD_SUPP_OBJ = "/5/0/4";
Lwm2m.prototype.LWM2M_URI_FIRMWARE_UPDATE_RES = "/5/0/5";
Lwm2m.prototype.LWM2M_URI_FIRMWARE_PKG_NAME = "/5/0/6";
Lwm2m.prototype.LWM2M_URI_FIRMWARE_PKG_URI = "/5/0/7";
Lwm2m.prototype.LWM2M_URI_LOCATION_LATITUDE = "/6/0/0";
Lwm2m.prototype.LWM2M_URI_LOCATION_LONGITUDE = "/6/0/1";
Lwm2m.prototype.LWM2M_URI_LOCATION_ALTITUDE = "/6/0/2";
Lwm2m.prototype.LWM2M_URI_LOCATION_UNCERTAINTY = "/6/0/3";
Lwm2m.prototype.LWM2M_URI_LOCATION_VELOCITY = "/6/0/4";
Lwm2m.prototype.LWM2M_URI_LOCATION_TIMESTAMP = "/6/0/5";

Lwm2m.prototype.LWM2M_FIRMWARE_STATE_IDLE = "0";
Lwm2m.prototype.LWM2M_FIRMWARE_STATE_DOWNLOADING = "1";
Lwm2m.prototype.LWM2M_FIRMWARE_STATE_DOWNLOADED = "2";
Lwm2m.prototype.LWM2M_FIRMWARE_STATE_UPDATING = "3";

Lwm2m.prototype.LWM2M_FIRMWARE_UPD_RES_DEFAULT = "0";
Lwm2m.prototype.LWM2M_FIRMWARE_UPD_RES_SUCCESS = "1";
Lwm2m.prototype.LWM2M_FIRMWARE_UPD_RES_SPACE_ERR = "2";
Lwm2m.prototype.LWM2M_FIRMWARE_UPD_RES_OOM = "3";
Lwm2m.prototype.LWM2M_FIRMWARE_UPD_RES_CONNE_ERR = "4";
Lwm2m.prototype.LWM2M_FIRMWARE_UPD_RES_CRC_ERR = "5";
Lwm2m.prototype.LWM2M_FIRMWARE_UPD_RES_PKG_ERR = "6";
Lwm2m.prototype.LWM2M_FIRMWARE_UPD_RES_URI_ERR = "7";

Lwm2m.prototype.client_request = function(id, uri, name, lifetime,
        connect_timeout, objects, psk_id, psk_key, certificate_mode_config) {
    var _ = this;
    return this.lwm2m.client_request(id, uri, name, lifetime, connect_timeout,
            objects, psk_id, psk_key, certificate_mode_config,
            function(err) {
                _.emit('error', err);
            },
            function(uri) {
                _.emit('execute', uri);
            },
            function(uri, buffer) {
                _.emit('changed', uri, buffer);
            });
}

Lwm2m.prototype.client_release = function() {
    return this.lwm2m.client_release();
}

Lwm2m.prototype.client_connect = function() {
    return this.lwm2m.client_connect();
}

Lwm2m.prototype.client_disconnect = function() {
    return this.lwm2m.client_disconnect();
}

Lwm2m.prototype.client_write_resource = function(uri, buffer) {
    return this.lwm2m.client_write_resource(uri, buffer);
}

Lwm2m.prototype.client_read_resource = function(uri) {
    return this.lwm2m.client_read_resource(uri);
}

Lwm2m.prototype.serialize_tlv_int = function(array_data) {
    return this.lwm2m.serialize_tlv_int(array_data);
}

Lwm2m.prototype.serialize_tlv_string = function(array_data) {
    return this.lwm2m.serialize_tlv_string(array_data);
}
