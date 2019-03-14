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

const PublishTopic = 'command';

// TODO Liet ke cac command
const ZigbeeCommand = {
    CreateNetwork: 'plugin network-creator start',
    PermitJoin: '',
    PluginDeviceTable: 'plugin device-table',
    OnOff: 'zcl on-off',
};

// TODO Taibeo Liet ke cac clusterId
const ZigbeeCluster = {
    BASIC: {
        ID: '0x0000',
        Attribute: {
            ZCL_VERSION_ATTRIBUTE_ID: {
                ID: '0002',
                type: '20'
            },
            ZCL_MANUFACTURER_NAME_ATTRIBUTE_ID: {
                ID: '0004',
                type: '42'
            },
            ZCL_MODEL_IDENTIFIER_ATTRIBUTE_ID: {
                ID: '0005',
                type: '42'
            },
            ZCL_POWER_SOURCE_ATTRIBUTE_ID: {
                ID: '0007',
                type: '30'
            },
            ZCL_BASIC_CLUSTER_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID: {
                ID: 'FFFD',
                type: '21'
            }
        }
    },
    ONOFF: {
        ID: '0x0006',
        Attribute: {
            ZCL_ON_OFF_ATTRIBUTE_ID: {
                ID: '0000',
                type: '10'
            },
            ZCL_ON_OFF_CLUSTER_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID: {
                ID: 'FFFD',
                type: '21'
            }
        },
        Command: {
            On: '01',
            Off: '00',
            Toggle: '02',
            OffWithEffect: '40',
            OnWithTimeOff: '42'
        }
    },
    IDENTIFY: {
        ID: '0x0003',
        Attribute: {
            ZCL_IDENTIFY_TIME_ATTRIBUTE_ID:{
                ID: '0000',
                type: '21'
            },
            ZCL_COMMISSION_STATE_ATTRIBUTE_ID: {
                ID: '0001',
                type: '18'
            },
            ZCL_IDENTIFY_CLUSTER_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID: {
                ID: 'FFFD',
                type: '21'
            }
        },
        Command: {
            Identify: '00',
            IdentifyQuery: '01'
        }
    },
    LEVELCONTROL: {
        ID: '0x0008',
        Attribute: {
            ZCL_CURRENT_LEVEL_ATTRIBUTE_ID: {
                ID: '0000',
                type: '20'
            },
            ZCL_LEVEL_CONTROL_REMAINING_TIME_ATTRIBUTE_ID: {
                ID: '0001',
                type: '21'
            },
            ZCL_ON_LEVEL_ATTRIBUTE_ID: {
                ID: '0011',
                type: '20'
            },
            ZCL_LEVEL_CONTROL_CLUSTER_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID: {
                ID: 'FFFD',
                type: '21'
            }
        },
        Command: {
            MoveToLevel: '00',
            Move: '01',
            Step: '02',
            Stop: '03',
            MoveToLevelWithOnOff: '04',
            MoveWithOnOff: '05',
            StepWithOnOff: '06',
            StopWithOnOff: '07'
        }

    }
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
            logger.info('RECEIVE ' + topicLevel[2] + ': ' + message);
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
        logger.info('SEND ' + PublishTopic + ': ' + message);
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
                    command: params.on ? ZigbeeCommand.OnOff + ' on' : ZigbeeCommand.OnOff + ' off',
                    postDelayMs: postTimeDelay
                });
                let eui64 = params.eui64.split('x');
                payload.commands.push({
                    command: ZigbeeCommand.PluginDeviceTable + ' send ' + eui64[1] + ' ' + params.endpoint,
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

    //TODO Taibeo Get command value from cluster ID and command data
     static _parseClusterValue(clusterId, commandData) {
        let value = {};
        switch (clusterId) {
            // TODO Taibeo Switch theo cac case cuar Attribute duoc liet ke o tren
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
            case ZigbeeCluster.BASIC.ID: {

            } break;
        }
        return value;
    };
}

module.exports = { ZigbeeGateway, ZigbeeCommand};