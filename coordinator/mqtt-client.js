/**
 * Author: TaiNV
 * Date Created: 2019/02/16
 * Module: mqtt-client
 * Description: provide interprocess communication by using mqtt
 */

// Depedencies
const logging = require('../libraries/system-log').create(__filename);
const mqtt = require('mqtt');
const deviceManager = require('../controller/device-manager/device-manager');
const helpers = require('../libraries/helpers');

// Container for all methods
const mqttClient = {};

const _clientID = helpers.createRandomString(20);

var _clientOptions = {
    port: 1883,
    host: 'localhost',
    clientId: _clientID,
    protocol: 'mqtt'
};

mqttClient.zigbee_topics = {
    subscribe: {
        device_joined: 'gw/000B57FFFE572517/devicejoined',
        device_left: 'gw/000B57FFFE572517/deviceleft',
        zcl_response: 'gw/000B57FFFE572517/zclresponse',
    },
    publish: {
        command: 'gw/000B57FFFE572517/command'
    }
}

mqttClient.system_topics = {
    device_config: _clientID + "/devices/config",
    device_status: _clientID + "/devices/status",
    device_control: _clientID + "/devices/control",
    rule_config: _clientID + "/rules/config",
    rule_active: _clientID + "/rules/active"
}

mqttClient.publish = (topic, message) => {
    _client.publish(topic, message, err => {
        if (err) {
            logging.error("Client is disconnecting...");
        }
    });
}

mqttClient.connect = () => {

}

const _client = mqtt.connect(_clientOptions);

client.on('connect', connack => {
    client.subscribe();
});

client.on('message', (topic, message) => {
    logging.info(topic + ': ' + message);
});