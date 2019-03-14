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
    // FAN: '0x',
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
                this.status.on = value;
                let payload = ZigbeeGateway.createZigbeeCommand(ZigbeeCommand.OnOff, {
                    eui64: this.eui64,
                    endpoint: this.endpoint,
                    on: value
                });
                zigbeePublisher.publish(payload);
                logger.debug("Published");
                callback();
            })
            .on('get', callback => {
                logger.debug("Get status");
                callback(null, this.status.on)
            })
    }

    get name() {
        return `${this.eui64}_${this.endpoint}`;
    }

    updateValue(value) {
        this.getCharacteristic(Characteristic.On)
            .updateValue(value, undefined);
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
                logger.debug("Get status");
                callback(null, this.status.on)
            });
        this.getCharacteristic(Characteristic.Brightness)
            .on('set', (value, callback) => {
                logger.info("Set characteristic brightness of service " + this.name);
                console.log("Set characteristic brightness of service " + this.name);
                this.status.brightness = value;
                // TODO Create level control command
                let payload = ZigbeeGateway.createZigbeeCommand();
                zigbeePublisher.publish(payload);
                callback();
            })
            .on('get', callback => {
                logger.debug("Get status");
                callback(this.status.brightness);
            })
    }

    get name() {
        return `${this.eui64}_${this.endpoint}`;
    }
}

module.exports = { EndpointType, SwitchEndpoint, LightEndpoint };