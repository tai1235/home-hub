/**
 * Author: TaiNV
 * Date Created: 2019/02/16
 * Module: zigbee-gateway
 * Description: Zigbee Gateway communication using MQTT implementation
 */

// Depedencies
const EventEmitter = require('events');
const Logger = require('../../libraries/system-log');
const mqtt = require('mqtt');
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
    }

    start() {
        this.client = mqtt.connect({
            host: 'localhost',
            port: 1883,
            protocol: 'mqtt'
        });
    }

    onConnect(callback) {
        logger.info('START mqtt client');
        this.client.on('connect', () => {
            logger.debug("Connected to broker");
            this.client.subscribe([
                'gw/' + this.eui64 + '/' + SubscribeTopics.DeviceJoined,
                'gw/' + this.eui64 + '/' + SubscribeTopics.DeviceLeft,
                'gw/' + this.eui64 + '/' + SubscribeTopics.ZclResponse
            ]);
            // Form network
            let command = ZigbeeGateway.createZigbeeCommand(ZigbeeCommand.Plugin.FormNetwork);
            this.publish(command);
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
                    this.emit('zigbee-device-joined', params);
                } break;
                case SubscribeTopics.DeviceLeft: {
                    this.emit('zigbee-device-left', messageData.eui64);
                } break;
                case SubscribeTopics.ZclResponse: {
                    let params = {
                        eui64: messageData.deviceEndpoint.eui64,
                        endpoint: messageData.deviceEndpoint.endpoint,
                        value: ZigbeeGateway._parseClusterValue(messageData.clusterId, messageData.commandData, messageData.commandId)
                    };
                    this.emit('zigbee-device-response', params);
                } break;
            }
        })
    }

    publish(message) {
        logger.debug('SEND ' + PublishTopic + ': ' + message);
        let topic = 'gw/' + this.eui64 + '/' + PublishTopic;
        this.client.publish(topic, message);
    }

    static createZigbeeCommand(cmd, params = {}, postTimeDelay = 0) {
        let payload = {};
        payload.commands = [];
        switch (cmd) {
            case ZigbeeCommand.Plugin.FormNetwork: {
                /**
                 * {number} params.boolean - Whether or not to form a centralized network. Should let it be true.
                 */
                payload.commands.push({
                    command: ZigbeeCommand.Plugin.FormNetwork + ' 1',
                    postDelayMs : postTimeDelay
                })
            } break;
            case ZigbeeCommand.Plugin.LeaveNetwork: {
                /**
                 * NONE
                 */
                payload.commands.push({
                    command: ZigbeeCommand.Plugin.LeaveNetwork,
                    postDelayMs : postTimeDelay
                })
            } break;
            case ZigbeeCommand.OnOff.On: {
                let eui64 = params.eui64.split('x')[1];
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
                let eui64 = params.eui64.split('x')[1];
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
                let eui64 = params.eui64.split('x')[1];
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
                 * NONE
                 */
                payload.commands.push({
                    command: ZigbeeCommand.Plugin.PermitJoin,
                    postDelayMs: postTimeDelay
                })
            } break;
            case ZigbeeCommand.Plugin.PermitStopJoin: {
                /**
                 * NONE
                 */
                payload.commands.push({
                    command: ZigbeeCommand.Plugin.PermitStopJoin,
                    postDelayMs: postTimeDelay
                })
            } break;
            default:
            break;
        }
        return JSON.stringify(payload);
    }

     static _parseClusterValue(clusterId, commandData, commandId) {
        if (commandData === undefined)
            return {};

        let value = {};

        // Get attribute id, type and data
        let attributeID = commandData.substr(4, 2) + commandData.substr(2, 2);
        let attributeType = commandData.substr(8, 2);
        let attributeData = commandData.substr(10);
        if (commandId === '0x0A') {
            attributeType = commandData.substr(6, 2);
            attributeData = commandData.substr(8);
        }

        // Validate attribute data
        if (attributeData.length >= 2) {
            attributeData = attributeData.match(/.{1,2}(?=(.{2})+(?!.))|.{1,2}$/g).reverse().join('');
        } else {
            return {};
        }

        switch (clusterId) {
            // Using switch for each ClusterID for addition of Attribute in future
            case ZigbeeCluster.BASIC.ID: {
                switch(attributeID){
                    case ZigbeeCluster.BASIC.Attribute.MANUFACTURER_NAME.ID: {
                        if (attributeType === ZigbeeCluster.BASIC.Attribute.MANUFACTURER_NAME.type) {
                            value.manufacturer = ZigbeeGateway.hex2string(attributeData);
                        }
                    } break;
                    case ZigbeeCluster.BASIC.Attribute.MODEL_IDENTIFIER.ID:{
                        if (attributeType === ZigbeeCluster.BASIC.Attribute.MODEL_IDENTIFIER.type) {
                            value.model = ZigbeeGateway.hex2string(attributeData);
                        }
                    } break;
                    default: break;
                }
            } break;
            case ZigbeeCluster.ON_OFF.ID: {
                switch (attributeID) {
                    case ZigbeeCluster.ON_OFF.Attribute.ON_OFF.ID: {
                        if (attributeType === ZigbeeCluster.ON_OFF.Attribute.ON_OFF.type) {
                            if (commandId === '0x0A') {
                                if (parseInt(attributeData, 16) === 0) {
                                    value.act = 0;
                                }
                            } else {
                                value.on = parseInt(attributeData, 16) === 1;
                            }
                        }
                    } break;
                    case ZigbeeCluster.ON_OFF.Attribute.PEBBLE.ID: {
                        if (attributeType === ZigbeeCluster.ON_OFF.Attribute.PEBBLE.type) {
                            if (parseInt(attributeData) === 2) {
                                value.act = 1;
                            }
                        }
                    } break;
                    default: break;
                }
            } break;
            case ZigbeeCluster.LEVEL_CONTROL.ID: {
                switch (attributeID) {
                    case ZigbeeCluster.LEVEL_CONTROL.Attribute.CURRENT_LEVEL.ID: {
                        if (attributeType === ZigbeeCluster.LEVEL_CONTROL.Attribute.CURRENT_LEVEL.type) {
                            value.birghtness = (parseInt(attributeData, 16) * 100 / 255).toFixed(0);
                        }
                    }
                }
            } break;
            case ZigbeeCluster.ILLUM_MEASUREMENT.ID: {
                switch (attributeID) {
                    case ZigbeeCluster.ILLUM_MEASUREMENT.Attribute.ILLUM_MEASURED_VALUE.ID: {
                        if (attributeType === ZigbeeCluster.ILLUM_MEASUREMENT.Attribute.ILLUM_MEASURED_VALUE.type) {
                            value.lux = parseInt(attributeData, 16);
                        }
                    } break;
                    default: break;
                }
            } break;
            case ZigbeeCluster.RELATIVE_HUMIDITY_MEASUREMENT.ID: {
                switch (attributeID) {
                    case ZigbeeCluster.RELATIVE_HUMIDITY_MEASUREMENT.Attribute.RELATIVE_HUMIDITY_MEASURED_VALUE.ID: {
                        if (attributeType === ZigbeeCluster.RELATIVE_HUMIDITY_MEASUREMENT.Attribute.RELATIVE_HUMIDITY_MEASURED_VALUE.type) {
                            value.humidity = parseInt(attributeData, 16);
                        }
                    } break;
                    default: break;
                }
            } break;
            case ZigbeeCluster.TEMP_MEASUREMENT.ID: {
                switch (attributeID) {
                    case ZigbeeCluster.TEMP_MEASUREMENT.Attribute.TEMP_MEASURED_VALUE.ID: {
                        if (attributeType === ZigbeeCluster.TEMP_MEASUREMENT.Attribute.TEMP_MEASURED_VALUE.type) {
                            value.temperature = parseInt(attributeData, 16);
                            // TODO Careful with signed int data type
                        }
                    } break;
                    default: break;
                }
            } break;
            case ZigbeeCluster.IAS_ZONE.ID: {
                switch (attributeID) {
                    case ZigbeeCluster.IAS_ZONE.Attribute.ZONE_STATUS.ID: {
                        if (attributeType === ZigbeeCluster.IAS_ZONE.Attribute.ZONE_STATUS.type) {
                            value.motion = parseInt(attributeData, 16) === 1;
                        }
                    } break;
                    default: break;
                }
            } break;
            case ZigbeeCluster.DOOR_LOCK.ID: {
                switch (attributeID) {
                    case ZigbeeCluster.DOOR_LOCK.Attribute.DOOR_STATE.ID: {
                        if (attributeType === ZigbeeCluster.DOOR_LOCK.Attribute.DOOR_STATE.type) {
                            value.contact = parseInt(attributeData, 16) === 0;
                        }
                    } break;
                    default: break;
                }
            } break;
            case ZigbeeCluster.POWER_CONFIG.ID: {
                switch (attributeID) {
                    case ZigbeeCluster.POWER_CONFIG.Attribute.BATTERY_PERCENTAGE_REMAINING.ID: {
                        if (attributeType === ZigbeeCluster.POWER_CONFIG.Attribute.BATTERY_PERCENTAGE_REMAINING.type) {
                            value.battery = parseInt(attributeData, 16);
                        }
                    } break;
                    default: break;
                }
            } break;
            default: break;
        }

        return value;
    };

    static hex2string(hex) {
        let input = hex.toString(); //force conversion
        let str = '';
        for (let i = 0; (i < input.length && input.substr(i, 2) !== '00'); i += 2)
            str += String.fromCharCode(parseInt(input.substr(i, 2), 16));
        return str;
    };
}

module.exports = ZigbeeGateway;