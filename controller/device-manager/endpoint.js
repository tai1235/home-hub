/**
 * Author: TaiNV
 * Date Modified: 2019/03/31
 * Module: endpoint
 * Description: all endpoint's types declarations and interfaces
 */

// Dependencies
let Service = require('../../hap').Service;
let Characteristic = require('../../hap').Characteristic;
let ZigbeeGateway = require('../../coordinator/zigbee/zigbee-gateway');
let ZigbeeCommand = require('../../coordinator/zigbee/zigbee').ZigbeeCommand;
let Logger = require('../../libraries/system-log');
let config = require('../../config');

let logger = new Logger(__filename);
let zigbeePublisher = new ZigbeeGateway(config.gatewayId);

const EndpointType = {
    SWITCH: '0x0100',
    LIGHT: '0x0103',
    CONTACT_SENSOR: '0x000A',
    MOTION_SENSOR: '0x0403',
    LIGHT_SENSOR: '0x0106',
    TEMPERATURE_SENSOR: '0x0302',
    HUMIDITY_SENSOR: '0x0301',
    BATTERY: '0x0017',
    PEBBLE: '',
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
                callback(null, this.status.brightness);
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

class ContactSensorEndpoint extends Service.ContactSensor {
    constructor(eui64, endpoint) {
        super(eui64 + '_' + endpoint, endpoint);
        this.eui64 = eui64;
        this.endpoint = endpoint;
        this.status = { state: 1 };
        this.getCharacteristic(Characteristic.ContactSensorState)
            .on('get', callback => {
                logger.info('GET characteristic contact sensor state of ' + this.name + ': ' + this.status.state);
                callback(null, this.status.state)
            })
    }

    get name() {
        return `${this.eui64}_${this.endpoint}`;
    }

    updateValue(value) {
        if (value.state !== undefined) {
            logger.info('UPDATE characteristic contact sensor state of ' + this.name + ': ' + value.state);
            this.status.state = value.state;
            this.getCharacteristic(Characteristic.ContactSensorState)
                .updateValue(value.state, undefined);
        }
    }
}

class MotionSensorEndpoint extends Service.MotionSensor {
    constructor(eui64, endpoint) {
        super(eui64 + '_' + endpoint, endpoint);
        this.eui64 = eui64;
        this.endpoint = endpoint;
        this.status = { state: 1 };
        this.getCharacteristic(Characteristic.MotionDetected)
            .on('get', callback => {
                logger.info('GET characteristic motion detected of ' + this.name + ': ' + this.status.state);
                callback(null, this.status.state)
            })
    }

    get name() {
        return `${this.eui64}_${this.endpoint}`;
    }

    updateValue(value) {
        if (value.state !== undefined) {
            logger.info('UPDATE characteristic motion detected of ' + this.name + ': ' + value.state);
            this.status.state = value.state;
            this.getCharacteristic(Characteristic.MotionDetected)
                .updateValue(value.state, undefined);
        }
    }
}

class LightSensorEndpoint extends Service.LightSensor {
    constructor(eui64, endpoint) {
        super(eui64 + '_' + endpoint, endpoint);
        this.eui64 = eui64;
        this.endpoint = endpoint;
        this.status = { level: 0 };
        this.getCharacteristic(Characteristic.CurrentAmbientLightLevel)
            .on('get', callback => {
                logger.info('GET characteristic current ambient light level of ' + this.name + ': ' + this.status.level);
                callback(null, this.status.level)
            })
    }

    get name() {
        return `${this.eui64}_${this.endpoint}`;
    }

    updateValue(value) {
        if (value.level !== undefined) {
            logger.info('UPDATE characteristic current ambient light level of ' + this.name + ': ' + value.level);
            this.status.level = value.level;
            this.getCharacteristic(Characteristic.CurrentAmbientLightLevel)
                .updateValue(value.level, undefined);
        }
    }
}

class TemperatureSensorEndpoint extends Service.TemperatureSensor {
    constructor(eui64, endpoint) {
        super(eui64 + '_' + endpoint, endpoint);
        this.eui64 = eui64;
        this.endpoint = endpoint;
        this.status = { level: 0 };
        this.getCharacteristic(Characteristic.CurrentTemperature)
            .on('get', callback => {
                logger.info('GET characteristic current temperature of ' + this.name + ': ' + this.status.level);
                callback(null, this.status.level)
            })
    }

    get name() {
        return `${this.eui64}_${this.endpoint}`;
    }

    updateValue(value) {
        if (value.level !== undefined) {
            logger.info('UPDATE characteristic current temperature of ' + this.name + ': ' + value.level);
            this.status.level = value.level;
            this.getCharacteristic(Characteristic.CurrentTemperature)
                .updateValue(value.level, undefined);
        }
    }
}

class HumiditySensorEndpoint extends Service.HumiditySensor {
    constructor(eui64, endpoint) {
        super(eui64 + '_' + endpoint, endpoint);
        this.eui64 = eui64;
        this.endpoint = endpoint;
        this.status = {level: 0};
        this.getCharacteristic(Characteristic.CurrentRelativeHumidity)
            .on('get', callback => {
                logger.info('GET characteristic current relative humidity of ' + this.name + ': ' + this.status.level);
                callback(null, this.status.level)
            })
    }

    get name() {
        return `${this.eui64}_${this.endpoint}`;
    }

    updateValue(value) {
        if (value.level !== undefined) {
            logger.info('UPDATE characteristic current relative humidity of ' + this.name + ': ' + value.level);
            this.status.level = value.level;
            this.getCharacteristic(Characteristic.CurrentRelativeHumidity)
                .updateValue(value.level, undefined);
        }
    }
}

class BatteryEndpoint extends Service.BatteryService {
    constructor(eui64, endpoint) {
        super(eui64 + '_' + endpoint, endpoint);
        this.eui64 = eui64;
        this.endpoint = endpoint;
        this.status = {level: 0};
        this.getCharacteristic(Characteristic.BatteryLevel)
            .on('get', callback => {
                logger.info('GET characteristic battery level of ' + this.name + ': ' + this.status.level);
                callback(null, this.status.level)
            })
    }

    get name() {
        return `${this.eui64}_${this.endpoint}`;
    }

    updateValue(value) {
        if (value.level !== undefined) {
            logger.info('UPDATE characteristic battery level of ' + this.name + ': ' + value.level);
            this.status.level = value.level;
            this.getCharacteristic(Characteristic.BatteryLevel)
                .updateValue(value.level, undefined);
        }
    }
}

module.exports = {
    EndpointType,
    SwitchEndpoint,
    LightEndpoint,
    ContactSensorEndpoint,
    MotionSensorEndpoint,
    LightSensorEndpoint,
    TemperatureSensorEndpoint,
    HumiditySensorEndpoint,
    BatteryEndpoint
};