/* Platform exports */
module.exports.artik520 = require('./platform/artik520');
module.exports.artik710 = require('./platform/artik710');
module.exports.artik1020 = require('./platform/artik1020');
module.exports.artik530 = require('./platform/artik530');
module.exports.artik305 = require('./platform/artik305');
module.exports.eagleye530 = require('./platform/eagleye530');

/* Addon exports */
var artik = require('../build/Release/artik-sdk.node');

module.exports.get_modules = artik.get_modules;
module.exports.get_platform = artik.get_platform;
module.exports.get_platform_name = artik.get_platform_name;
module.exports.get_device_info = artik.get_device_info;
module.exports.get_bt_mac_address = artik.get_bt_mac_address;
module.exports.get_wifi_mac_address = artik.get_wifi_mac_address;
module.exports.get_platform_serial_number = artik.get_platform_serial_number;
module.exports.get_platform_manufacturer = artik.get_platform_manufacturer;
module.exports.get_platform_uptime = artik.get_platform_uptime;
module.exports.get_platform_model_number = artik.get_platform_model_number;
module.exports.destroy = artik.destroy;

module.exports.adc = artik.adc;
module.exports.i2c = artik.i2c;
module.exports.media = artik.media;
module.exports.pwm = artik.pwm;
module.exports.sensor = artik.sensor;
module.exports.spi = artik.spi;

/* Other exports */
module.exports.time = require('./time');
module.exports.http = require('./http');
module.exports.bluetooth = require('./bluetooth');
module.exports.wifi = require('./wifi');
module.exports.zigbee = require('./zigbee');
module.exports.gpio = require('./gpio');
module.exports.serial = require('./serial');
module.exports.websocket = require('./websocket');
module.exports.lwm2m = require('./lwm2m');
module.exports.mqtt = require('./mqtt');
module.exports.cloud = require('./cloud');
module.exports.network = require('./network');
module.exports.security = require('./security');
