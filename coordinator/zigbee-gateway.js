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
    let attirbute = {};
    switch (clusterId) {
        case '0x0000': {

        } break;
        case '0x0006': {

            attirbute.id = messageData.commandData.substring(2, 5);
            attirbute.dataType = messageData.commandData.substring(6, 7);
            attirbute.dataValue = messageData.commandData.substring(7, 8)
        } break;
    }
    return value;
};


// Container for all methods
class ZigbeeGateway extends EventEmitter {
    constructor(eui64) {
        super();
        this.eui64 = eui64;
        this.clientId = helpers.createRandomString(32);
        this.client = mqtt.connect({
            host: '192.168.11.79',
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
            ], (err, granted) => {
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
            if (topicLevel[2] === SubscribeTopics.DeviceJoined) {
                // TODO Parse eui64 from message
                // TODO Parse number of endpoint (fixed / from message)
                // TODO Parse device information ()
                // TODO Pass device's data to the event emitter
                this.emit('device-joined', message)
            } else if (topicLevel[2] === SubscribeTopics.DeviceLeft) {
                this.emit('device-left', message)
            } else if (topicLevel[2] === SubscribeTopics.ZclResponse) {
                if (messageData.clusterId === '0x0006') {

                }
                this.emit('device-status', message)
            }
        })
    }

    publish(message) {
        logger.info('Gateway[' + PublishTopic + ']' + ': ' + message);
        let topic = 'gw/' + this.eui64 + '/' + PublishTopic;
        this.client.publish(topic, message)
    }

    static createZigbeeCommand(cmd, params, postTimeDelay = 0) {
        let payload = {};
        payload.commands = [];
        switch (cmd) {
            case ZigbeeCommand.OnOff: {
                let eui64 = params.eui64;
                let onOff = params.on === 1 ? 'on' : 'off';
                let deviceEndpoint = params.deviceEndpoint;
                let commandItem1 = {
                    command: ZigbeeCommand.OnOff + ' ' + onOff,
                    postDelayMs: postTimeDelay
                };
                payload.commands.push(commandItem1);
                let commandItem2 = {
                    command: ZigbeeCommand.PluginDeviceTable + ' send ' + eui64 + ' ' + deviceEndpoint,
                    postDelayMs: postTimeDelay
                };
                payload.commands.push(commandItem2);
            } break;
            case ZigbeeCommand.CreateNetwork: {

            } break;
        }
        return payload
    }
}

module.exports = { ZigbeeGateway, ZigbeeCommand};