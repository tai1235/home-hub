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

// TODO Liet ke cac command
const ZigbeeCommand = {
    CreateNetwork: 'plugin network-creator start',
    PermitJoin: '',
    PluginDeviceTable: 'plugin device-table',
    OnOff: 'zcl on-off',
};

const PublishTopic = 'command';

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
                    command: params.on ? ZigbeeCommand.OnOff + ' on' : ZigbeeCommand.OnOff + ' off',
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

    //TODO Taibeo Get command value from cluster ID and command data
    static _parseClusterValue(clusterId, commandData) {
        let attribute = {};
        switch (clusterId) {
            // TODO Taibeo Switch theo cac case cuar Attribute duoc liet ke o tren
            case ZigbeeCluster.ONOFF.ID: {
                switch (commandData.substring(2, 5)) {
                    case ZigbeeCluster.ONOFF.Attribute.ZCL_ON_OFF_ATTRIBUTE_ID.ID: {
                        if (commandData.substring(6, 7) !== ZigbeeCluster.ONOFF.Attribute.ZCL_ON_OFF_ATTRIBUTE_ID.type) {
                            return false;
                        } else {
                            attribute.id = ZigbeeCluster.ONOFF.Attribute.ZCL_ON_OFF_ATTRIBUTE_ID.ID;
                            attribute.dataType = ZigbeeCluster.ONOFF.Attribute.ZCL_ON_OFF_ATTRIBUTE_ID.type;
                            attribute.dataValue = commandData.substring(7, 8) === '01' ? true : false;
                        }
                    } break;
                    case ZigbeeCluster.ONOFF.Attribute.ZCL_ON_OFF_CLUSTER_CLUSTER_REVISION_SERVER_ATTRIBUTE_ID.ID {

                    } break;
                    default: {

                    } break;
                }
            } break;
            case ZigbeeCluster.BASIC.ID: {
                
            } break;
        }
        return attribute;
    };
}

module.exports = { ZigbeeGateway, ZigbeeCommand};