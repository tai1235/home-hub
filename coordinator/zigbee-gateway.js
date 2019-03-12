/**
 * Author: TaiNV
 * Date Created: 2019/02/16
 * Module: zigbee-gateway
 * Description: Zigbee Gateway communication using MQTT implementation
 */

// Depedencies
const EventEmitter = require('events');
const Logger = require('../libraries/system-log');
const mqtt = require('mqtt');
const helpers = require('../libraries/helpers');

const logger = new Logger(__filename);

const SubscribeTopics = {
    DeviceJoined: 'devicejoined',
    DeviceLeft: 'deviceleft',
    ZclResponse: 'zclresponse',
};

const ZigbeeCommand = {
    CreateNetwork: 'pluin network-creator start',
    PermitJoin: '',
    PluginDeviceTable: 'plugin device-table',
    OnOff: 'zcl on-off',
};

const PublishTopic = 'command';

const ClusterID = {
    BASIC: '0x0000',
    ONOFF: '0x0006'
};

const Attribute = {

};

//TODO Get command value from cluster ID and command data
_getClusterValue = (clusterId, commandData) => {
    let attribute = {};
    switch (clusterId) {
        case '0x0000': {

        } break;
        case '0x0006': {
            attribute.id = commandData.substring(2, 5);
            attribute.dataType = commandData.substring(6, 7);
            attribute.dataValue = commandData.substring(7, 8)
        } break;
    }
    return attribute;
};


// Container for all methods
class ZigbeeGateway extends EventEmitter {
    constructor(eui64) {
        super();
        this.eui64 = eui64;
        this.clientId = helpers.createRandomString(32);
        this.client = mqtt.connect({
            host: 'localhost',
            port: 1883,
            clientId: this.clientId,
            protocol: 'mqtt'
        })
    }

    getConnectStatus(callback) {
        this.client.on('connect', () => {
            logger.debug("Connected to broker");
            this.client.subscribe([
                'gw/' + this.eui64 + '/' + SubscribeTopics.DeviceJoined,
                'gw/' + this.eui64 + '/' + SubscribeTopics.DeviceLeft,
                'gw/' + this.eui64 + '/' + SubscribeTopics.ZclResponse
            ], (err) => {
                if (err)
                    logger.error("Lost connection to Zigbee Gateway");
                else {
                    logger.debug("Subscribe successfully");
                }
            });
            callback()
        })
    }

    process() {
        this.client.on('message', (topic, message) => {
            let topicLevel = topic.split('/');
            logger.info('Gateway[' + topicLevel[2] + ']' + ': ' + message);
            let messageData = JSON.parse(message.toString());
            switch (topicLevel[2]) {
                case SubscribeTopics.DeviceJoined: {
                    if (this.listener('device-joined').length > 0) {
                        // TODO Parse eui64 from message
                        // TODO Parse number of endpoint (fixed / from message)
                        // TODO Parse device information ()
                        // TODO Pass device's data to the event emitter
                        this.emit('device-joined', message);
                    }
                } break;
                case SubscribeTopics.DeviceLeft: {
                    if (this.listener('device-left').length > 0) {
                        this.emit('device-left', message);
                    }
                } break;
                case SubscribeTopics.ZclResponse: {
                    if (this.listener('device-response').length > 0) {
                        this.emit('device-response', message);
                    }
                } break;
            }
        })
    }

    publish(message) {
        logger.info('Gateway[' + PublishTopic + ']' + ': ' + message);
        let topic = 'gw/' + this.eui64 + '/' + PublishTopic;
        this.client.publish(topic, message);
    }

    static createZigbeeCommand(cmd, params, postTimeDelay = 0) {
        let payload = {};
        payload.commands = [];
        switch (cmd) {
            case ZigbeeCommand.OnOff: {
                /**
                 * {Object} params - Required params for this command
                 * {String} params.eui64 - Device used for this command
                 * {Number} params.endpoint - Endpoint to execute this command
                 * {Number} params.on - Value to assign to the endpoint
                 */
                payload.commands.push({
                    command: params.on === 1 ? ZigbeeCommand.OnOff + ' on' : ZigbeeCommand.OnOff + ' off',
                    postDelayMs: postTimeDelay
                });
                payload.commands.push({
                    command: ZigbeeCommand.PluginDeviceTable + ' send ' + params.eui64 + ' ' + params.endpoint,
                    postDelayMs: postTimeDelay
                });
            } break;
            case ZigbeeCommand.CreateNetwork: {
                /**
                 * {Object} params - Required params for this command
                 * {String} params.eui64 - Device used for this command
                 * {Number} params.endpoint - Endpoint to execute this command
                 * {Bool} params.on - Value to assign to the endpoint
                 */
            } break;
        }
        return JSON.stringify(payload);
    }
}

module.exports = { ZigbeeGateway, ZigbeeCommand};