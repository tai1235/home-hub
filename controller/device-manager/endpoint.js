// Dependencies
let Service = require('../../hap').Service;
let Characteristic = require('../../hap').Characteristic;
let ZigbeeGateway = require('../../coordinator/zigbee-gateway');
let ZigbeeCommand = require('../../coordinator/zigbee').ZigbeeCommand;
let Logger = require('../../libraries/system-log');
let config = require('../../config');

let logger = new Logger(__filename);
let zigbeePublisher = new ZigbeeGateway(config.gatewayId);

const EndpointType = {
    // TODO Taibeo Dien
    SWITCH: '0x0100',
    LIGHT: '0x0103',
    DOORLOCK: '0x000A',
    THERMOSTAT: '0x0301',
};

zigbeePublisher.start();

class SwitchEndpoint extends Service.Switch {
    constructor(eui64, endpoint) {
        super(eui64 + '_' + endpoint, endpoint);
        this.eui64 = eui64;
        this.endpoint = endpoint;
        this.status = { on: false };
        this.getCharacteristic(Characteristic.On)
            .on('set', (value, callback) => {
                logger.info('SET characteristic on of ' + this.name + ': ' + value);
                this.status.on = value;
                let command = value ? ZigbeeCommand.OnOff.On : ZigbeeCommand.OnOff.Off;
                let payload = ZigbeeGateway.createZigbeeCommand(command, {
                    eui64: this.eui64,
                    endpoint: this.endpoint,
                });
                zigbeePublisher.publish(payload);
                callback();
            })
            .on('get', callback => {
                logger.info('GET characteristic on of ' + this.name + ': ' + this.status.on);
                callback(null, this.status.on)
            })
    }

    get name() {
        return `${this.eui64}_${this.endpoint}`;
    }

    updateValue(value) {
        if (value.on !== undefined) {
            logger.info('UPDATE characteristic on of ' + this.name + ': ' + value.on);
            this.status.on = value.on;
            this.getCharacteristic(Characteristic.On)
                .updateValue(value.on, undefined);
        }
    }

    setValue(value) {
        if (value.on !== undefined) {
            logger.info('SET characteristic on of ' + this.name + ': ' + value.on);
            this.getCharacteristic(Characteristic.On)
                .setValue(value.on);
        }
    }
}

class LightEndpoint extends Service.Lightbulb {
    constructor(eui64, endpoint) {
        super(eui64 + '_' + endpoint, endpoint);
        this.eui64 = eui64;
        this.endpoint = endpoint;
        this.status = { on: false, brightness: 0 };
        this.getCharacteristic(Characteristic.On)
            .on('set', (value, callback) => {
                logger.info('SET characteristic on of ' + this.name + ': ' + value);
                this.status.on = value;
                let command = value ? ZigbeeCommand.OnOff.On : ZigbeeCommand.OnOff.Off;
                let payload = ZigbeeGateway.createZigbeeCommand(command, {
                    eui64: this.eui64,
                    endpoint: this.endpoint,
                    on: value
                });
                zigbeePublisher.publish(payload);
                callback();
            })
            .on('get', callback => {
                logger.info('GET characteristic on of ' + this.name + ': ' + this.status.on);
                callback(null, this.status.on)
            });
        this.getCharacteristic(Characteristic.Brightness)
            .on('set', (value, callback) => {
                logger.info('SET brightness value of ' + this.name + ': ' + value);
                this.status.brightness = value;
                let payload = ZigbeeGateway.createZigbeeCommand(ZigbeeCommand.LevelControl.MoveToLevel, {
                    eui64: this.eui64,
                    endpoint: this.endpoint,
                    level: value,
                    time: 0
                });
                zigbeePublisher.publish(payload);
                callback();
            })
            .on('get', callback => {
                logger.info('GET characteristic brightness of ' + this.name + ': ' + this.status.brightness);
                callback(this.status.brightness);
            })
    }

    get name() {
        return `${this.eui64}_${this.endpoint}`;
    }

    updateValue(value) {
        if (value.on !== undefined) {
            logger.info('UPDATE characteristic on of ' + this.name + ': ' + value.on);
            this.status.on = value.on;
            this.getCharacteristic(Characteristic.On)
                .updateValue(value.on, undefined);
        }
        if (value.level !== undefined) {
            logger.info('UPDATE characteristic brightness of ' + this.name + ': ' + value.brightness);
            this.status.brightness = value.level;
            this.getCharacteristic(Characteristic.Brightness)
                .updateValue(value.level, undefined);
        }
    }

    setValue(value) {
        if (value.on !== undefined) {
            logger.info('SET characteristic on of ' + this.name + ': ' + value.on);
            this.getCharacteristic(Characteristic.On)
                .setValue(value.on);
        }
        if (value.level !== undefined) {
            logger.info('SET characteristic on of ' + this.name + ': ' + value.on);
            this.getCharacteristic(Characteristic.Brightness)
                .setValue(value.level);
        }
    }
}

module.exports = { EndpointType, SwitchEndpoint, LightEndpoint };