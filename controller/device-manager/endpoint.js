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
let config = require('../../libraries/config');

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
    PEBBLE: '0x0104',
};

zigbeePublisher.start();

class SwitchEndpoint extends Service.Switch {
    constructor(eui64, endpoint) {
        super(eui64.substr(eui64.length - 6, eui64.length) + '-' + endpoint, endpoint);
        this.eui64 = eui64;
        this.endpoint = endpoint;
        this.type = EndpointType.SWITCH;
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
        return this.eui64.substr(this.eui64.length - 6) + '-' + this.endpoint;
    }

    updateValue(value) {
        logger.info('UPDATE characteristic on of ' + this.name + ': ' + value);
        this.status.on = value;
        this.getCharacteristic(Characteristic.On)
            .updateValue(value, undefined);
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
        super(eui64.substr(eui64.length - 6, eui64.length) + '-' + endpoint, endpoint);
        this.eui64 = eui64;
        this.endpoint = endpoint;
        this.type = EndpointType.LIGHT;
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
        return this.eui64.substr(this.eui64.length - 6) + '-' + this.endpoint;
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
        super(eui64.substr(eui64.length - 6, eui64.length) + '-' + endpoint, endpoint);
        this.eui64 = eui64;
        this.endpoint = endpoint;
        this.type = EndpointType.CONTACT_SENSOR;
        this.status = { contact: 0 };
        this.getCharacteristic(Characteristic.ContactSensorState)
            .on('get', callback => {
                logger.info('GET characteristic contact sensor state of ' + this.name + ': ' + this.status.contact);
                callback(null, this.status.contact)
            })
    }

    get name() {
        return this.eui64.substr(this.eui64.length - 6) + '-' + this.endpoint;
    }

    updateValue(value) {
        logger.info('UPDATE characteristic contact sensor state of ' + this.name + ': ' + value);
        this.status.contact = value;
        this.getCharacteristic(Characteristic.ContactSensorState)
            .updateValue(value, undefined);
    }
}

class MotionSensorEndpoint extends Service.MotionSensor {
    constructor(eui64, endpoint) {
        super(eui64.substr(eui64.length - 6, eui64.length) + '-' + endpoint, endpoint);
        this.eui64 = eui64;
        this.endpoint = endpoint;
        this.type = EndpointType.MOTION_SENSOR;
        this.status = { motion: 0 };
        this.getCharacteristic(Characteristic.MotionDetected)
            .on('get', callback => {
                logger.info('GET characteristic motion detected of ' + this.name + ': ' + this.status.motion);
                callback(null, this.status.motion)
            })
    }

    get name() {
        return this.eui64.substr(this.eui64.length - 6) + '-' + this.endpoint;
    }

    updateValue(value) {
        logger.info('UPDATE characteristic motion detected of ' + this.name + ': ' + value);
        this.status.motion = value;
        this.getCharacteristic(Characteristic.MotionDetected)
            .updateValue(value, undefined);
    }
}

class LightSensorEndpoint extends Service.LightSensor {
    constructor(eui64, endpoint) {
        super(eui64.substr(eui64.length - 6, eui64.length) + '-' + endpoint, endpoint);
        this.eui64 = eui64;
        this.endpoint = endpoint;
        this.type = EndpointType.LIGHT_SENSOR;
        this.status = { lux: 0 };
        this.getCharacteristic(Characteristic.CurrentAmbientLightLevel)
            .on('get', callback => {
                logger.info('GET characteristic current ambient light level of ' + this.name + ': ' + this.status.lux);
                callback(null, this.status.level)
            })
    }

    get name() {
        return this.eui64.substr(this.eui64.length - 6) + '-' + this.endpoint;
    }

    updateValue(value) {
        logger.info('UPDATE characteristic current ambient light level of ' + this.name + ': ' + value);
        this.status.lux = value;
        this.getCharacteristic(Characteristic.CurrentAmbientLightLevel)
            .updateValue(value, undefined);
    }
}

class TemperatureSensorEndpoint extends Service.TemperatureSensor {
    constructor(eui64, endpoint) {
        super(eui64.substr(eui64.length - 6, eui64.length) + '-' + endpoint, endpoint);
        this.eui64 = eui64;
        this.endpoint = endpoint;
        this.type = EndpointType.TEMPERATURE_SENSOR;
        this.status = { temperature: 0 };
        this.getCharacteristic(Characteristic.CurrentTemperature)
            .on('get', callback => {
                logger.info('GET characteristic current temperature of ' + this.name + ': ' + this.status.temperature);
                callback(null, this.status.temperature)
            })
    }

    get name() {
        return this.eui64.substr(this.eui64.length - 6) + '-' + this.endpoint;
    }

    updateValue(value) {
        logger.info('UPDATE characteristic current temperature of ' + this.name + ': ' + value);
        this.status.temperature = value;
        this.getCharacteristic(Characteristic.CurrentRelativeHumidity)
            .updateValue(value, undefined);
    }
}

class HumiditySensorEndpoint extends Service.HumiditySensor {
    constructor(eui64, endpoint) {
        super(eui64.substr(eui64.length - 6, eui64.length) + '-' + endpoint, endpoint);
        this.eui64 = eui64;
        this.endpoint = endpoint;
        this.type = EndpointType.HUMIDITY_SENSOR;
        this.status = { humidity: 0 };
        this.getCharacteristic(Characteristic.CurrentRelativeHumidity)
            .on('get', callback => {
                logger.info('GET characteristic current relative humidity of ' + this.name + ': ' + this.status.humidity);
                callback(null, this.status.humidity)
            })
    }

    get name() {
        return this.eui64.substr(this.eui64.length - 6) + '-' + this.endpoint;
    }

    updateValue(value) {
        logger.info('UPDATE characteristic current relative humidity of ' + this.name + ': ' + value);
        this.status.humidity = value;
        this.getCharacteristic(Characteristic.CurrentRelativeHumidity)
            .updateValue(value, undefined);
    }
}

class BatteryEndpoint extends Service.BatteryService {
    constructor(eui64, endpoint) {
        super(eui64.substr(eui64.length - 6, eui64.length) + '-' + endpoint, endpoint);
        this.eui64 = eui64;
        this.endpoint = endpoint;
        this.type = EndpointType.BATTERY;
        this.status = { battery: 100 };
        this.getCharacteristic(Characteristic.BatteryLevel)
            .on('get', callback => {
                logger.info('GET characteristic battery level of ' + this.name + ': ' + this.status.battery);
                callback(null, this.status.battery)
            })
    }

    get name() {
        return this.eui64.substr(this.eui64.length - 6) + '-' + this.endpoint;
    }

    updateValue(value) {
        logger.info('UPDATE characteristic battery level of ' + this.name + ': ' + value);
        this.status.battery = value;
        this.getCharacteristic(Characteristic.BatteryLevel)
            .updateValue(value, undefined);
    }
}

class PebbleEndpoint extends Service.StatelessProgrammableSwitch {
    constructor(eui64, endpoint) {
        super(eui64.substr(eui64.length - 6, eui64.length) + '-' + endpoint, endpoint);
        this.eui64 = eui64;
        this.endpoint = endpoint;
        this.type = EndpointType.PEBBLE;
    }

    get name() {
        return this.eui64.substr(this.eui64.length - 6) + '-' + this.endpoint;
    }

    updateValue(value) {
        logger.info('UPDATE characteristic programmable switch event of ' + this.name + ': ' + value);
        this.getCharacteristic(Characteristic.ProgrammableSwitchEvent)
            .updateValue(value, undefined);
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
    BatteryEndpoint,
    PebbleEndpoint
};