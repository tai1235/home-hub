// Dependencies
let Service = require('../../hap').Service;
let Characteristic = require('../../hap').Characteristic;
let { ZigbeeGateway, ZigbeeCommand } = require('../../coordinator/zigbee-gateway');
let Logger = require('../../libraries/system-log');
let config = require('../../config');

let logger = new Logger(__filename);
let zigbeePublisher = new ZigbeeGateway(config.gatewayId);

const EndpointType = {
    // TODO Taibeo Dien
    SWITCH: '0x0000',
    LIGHT: '0x0100',
    DOORLOCK: '0x000A',
    THERMOSTAT: '0x0301',

};

class SwitchEndpoint extends Service.Switch {
    constructor(eui64, endpoint) {
        super(eui64 + '_' + endpoint, endpoint);
        this.eui64 = eui64;
        this.endpoint = endpoint;
        this.status = { on: false };
        this.getCharacteristic(Characteristic.On)
            .on('set', (value, callback) => {
                logger.debug('SET on value of ' + this.name + ': ' + value);
                this.status.on = value;
                let payload = ZigbeeGateway.createZigbeeCommand(ZigbeeCommand.OnOff, {
                    eui64: this.eui64,
                    endpoint: this.endpoint,
                    on: value
                });
                zigbeePublisher.publish(payload);
                callback();
            })
            .on('get', callback => {
                logger.debug('GET on value of ' + this.name + ': ' + this.status.on);
                callback(null, this.status.on)
            })
    }

    get name() {
        return `${this.eui64}_${this.endpoint}`;
    }

    updateValue(value) {
        if (value.on !== undefined) {
            logger.debug('UPDATE on value of ' + this.name + ': ' + value.on);
            this.status.on = value.on;
            this.getCharacteristic(Characteristic.On)
                .updateValue(value.on, undefined);
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
                logger.debug('SET on value of ' + this.name + ': ' + value);
                this.status.on = value;
                let payload = ZigbeeGateway.createZigbeeCommand(ZigbeeCommand.OnOff, {
                    eui64: this.eui64,
                    endpoint: this.endpoint,
                    on: value
                });
                zigbeePublisher.publish(payload);
                callback();
            })
            .on('get', callback => {
                logger.debug('GET on value of ' + this.name + ': ' + this.status.on);
                callback(null, this.status.on)
            });
        this.getCharacteristic(Characteristic.Brightness)
            .on('set', (value, callback) => {
                logger.debug('SET brightness value of ' + this.name + ': ' + value);
                this.status.brightness = value;
                // TODO Create level control command
                let payload = ZigbeeGateway.createZigbeeCommand();
                zigbeePublisher.publish(payload);
                callback();
            })
            .on('get', callback => {
                logger.debug('GET brightness value of ' + this.name + ': ' + this.status.brightness);
                callback(this.status.brightness);
            })
    }

    get name() {
        return `${this.eui64}_${this.endpoint}`;
    }

    updateValue(value) {
        if (value.on !== undefined) {
            logger.debug('UPDATE on value of ' + this.name + ': ' + value.on);
            this.status.on = value.on;
            this.getCharacteristic(Characteristic.On)
                .updateValue(value.on, undefined);
        }
        if (value.level !== undefined) {
            logger.debug('UPDATE brightness value of ' + this.name + ': ' + value.brightness);
            this.status.brightness = value.level;
            this.getCharacteristic(Characteristic.Brightness)
                .updateValue(value.level, undefined);
        }
    }
}

module.exports = { EndpointType, SwitchEndpoint, LightEndpoint };