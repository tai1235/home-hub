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
const { ZigbeeCommand, ZigbeeCluster } = require('./zigbee');

const logger = new Logger(__filename);

const SubscribeTopics = {
    DeviceJoined: 'devicejoined',
    DeviceLeft: 'deviceleft',
    ZclResponse: 'zclresponse',
};

const PublishTopic = 'commands';

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
        logger.info('START mqtt client');
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
            logger.debug('RECEIVE ' + topicLevel[2] + ': ' + message);
            let messageData = JSON.parse(message.toString());
            switch (topicLevel[2]) {
                case SubscribeTopics.DeviceJoined: {
                    let params = {
                        type: messageData.deviceType,
                        eui64: messageData.deviceEndpoint.eui64,
                        endpoint: messageData.deviceEndpoint.endpoint
                    };
                    this.emit('device-joined', params);
                } break;
                case SubscribeTopics.DeviceLeft: {
                    this.emit('device-left', message);
                } break;
                case SubscribeTopics.ZclResponse: {
                    let params = {};
                    params.eui64 = messageData.deviceEndpoint.eui64;
                    params.endpoint = messageData.deviceEndpoint.endpoint;
                    params.value = ZigbeeGateway._parseClusterValue(messageData.clusterId, messageData.commandData);
                    this.emit('device-response', params);
                } break;
            }
        })
    }

    publish(message) {
        logger.debug('SEND ' + PublishTopic + ': ' + message);
        let topic = 'gw/' + this.eui64 + '/' + PublishTopic;
        this.client.publish(topic, message);
    }

    static createZigbeeCommand(cmd, params, postTimeDelay = 0) {
        let payload = {};
        payload.commands = [];
        let eui64 = params.eui64.split('x')[1];
        switch (cmd) {
            case ZigbeeCommand.Plugin.FormNetwork: {
                /**
                 * {Object} params - Required params for this command
                 * {String} params.eui64 - Device used for this command
                 * {Number} params.endpoint - Endpoint to execute this command
                 * {Number} params.level - Value to assign to the endpoint
                 */
            } break;
            case ZigbeeCommand.OnOff.On: {
                /**
                 * {Object} params - Required params for this command
                 * {String} params.eui64 - Device used for this command
                 * {Number} params.endpoint - Endpoint to execute this command
                 */
                payload.commands.push({
                    command: ZigbeeCommand.OnOff.On,
                    postDelayMs: postTimeDelay
                });
                payload.commands.push({
                    command: ZigbeeCommand.Plugin.DeviceTable + ' {' + eui64 + '} ' + params.endpoint,
                    postDelayMs: postTimeDelay
                });
            } break;
            case ZigbeeCommand.OnOff.Off: {
                /**
                 * {Object} params - Required params for this command
                 * {String} params.eui64 - Device used for this command
                 * {Number} params.endpoint - Endpoint to execute this command
                 */
                payload.commands.push({
                    command: ZigbeeCommand.OnOff.Off,
                    postDelayMs: postTimeDelay
                });
                payload.commands.push({
                    command: ZigbeeCommand.Plugin.DeviceTable + ' {' + eui64 + '} ' + params.endpoint,
                    postDelayMs: postTimeDelay
                });
            } break;
            case ZigbeeCommand.LevelControl.MoveToLevel: {
                /**
                 * {Object} params - Required params for this command
                 * {String} params.eui64 - Device used for this command
                 * {Number} params.endpoint - Endpoint to execute this command
                 * {Number} params.level - Level assign to this endpoint
                 * {Number} params.time - Transition time
                 */
                payload.commands.push({
                    command: ZigbeeCommand.LevelControl.MoveToLevel + ' ' + params.level + ' ' + params.time,
                    postDelayMs: postTimeDelay
                });
                payload.commands.push({
                    command: ZigbeeCommand.Plugin.DeviceTable + ' {' + eui64  + '} ' + params.endpoint,
                    postDelayMs: postTimeDelay
                })
            } break;
            case ZigbeeCommand.Plugin.PermitJoin: {
                /**
                 * {Object} params - Required params for this command
                 * {String} params.eui64 - Device used for this command
                 * {Number} params.endpoint - Endpoint to execute this command
                 * {Number} params.level - Value to assign to the endpoint
                 */
                payload.commands.push({
                    command: params
                })
            } break;
        }
        return JSON.stringify(payload);
    }

    //TODO Taibeo Get command value from cluster ID and command data
     static _parseClusterValue(clusterId, commandData) {
        if (commandData === undefined)
            return;
        let value = {};
        switch (clusterId) {
            // TODO Taibeo Switch theo cac case cuar Attribute duoc liet ke o tren
            case ZigbeeCluster.BASIC.ID: {

            } break;
            case ZigbeeCluster.ONOFF.ID: {
                switch (commandData.substr(2, 4)) {
                    case ZigbeeCluster.ONOFF.Attribute.ZCL_ON_OFF_ATTRIBUTE_ID.ID: {
                        if (commandData.substr(8, 2) !== ZigbeeCluster.ONOFF.Attribute.ZCL_ON_OFF_ATTRIBUTE_ID.type) {
                            return false;
                        } else {
                            value.on = commandData.substr(10, 2) === '01';
                        }
                    } break;
                    case ZigbeeCluster.ONOFF.Attribute.ZCL_ON_OFF_CLUSTER_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID.ID: {

                    } break;
                    default: {

                    } break;
                }
            } break;
        }
        return value;
    };

    static hex2string(hex) {
        let input = hex.toString();//force conversion
        let str = '';
        for (let i = 0; (i < input.length && input.substr(i, 2) !== '00'); i += 2)
            str += String.fromCharCode(parseInt(input.substr(i, 2), 16));
        return str;
    };
}

module.exports = ZigbeeGateway;