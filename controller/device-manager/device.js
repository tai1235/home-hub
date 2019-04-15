/**
 * Author: TaiNV
 * Date Created: 2019/02/26
 * Module: device
 * Description: device implementation
 */

// Dependencies
let Accessory = require('../../hap').Accessory;
let Service = require('../../hap').Service;
let Characteristic = require('../../hap').Characteristic;
let uuid = require('../../hap').uuid;
let Logger = require('../../libraries/system-log');
let Endpoint = require('../../controller/device-manager/endpoint');
let EndpointType = require('../../controller/device-manager/endpoint').EndpointType;

let logger = new Logger(__filename);

const DeviceType = {
    SWITCH: 'switch',
    LIGHT: 'light',
    CURTAIN: 'curtain',
    SENSOR: 'sensor'
};

// Device class
class Device extends Accessory {
    constructor(eui64, endpoint, type, serialNumber = '', manufacturer = '', model = '') {
        super(eui64, uuid.generate('hap-nodejs:accessories:' + type + eui64));
        this.eui64 = eui64;
        this.type = type;
        this.endpoints = [];
        if (serialNumber)
            this.getService(Service.AccessoryInformation)
                .setCharacteristic(Characteristic.SerialNumber, serialNumber);
        if (manufacturer)
            this.getService(Service.AccessoryInformation)
                .setCharacteristic(Characteristic.Manufacturer, manufacturer);
        if (model)
            this.getService(Service.AccessoryInformation)
                .setCharacteristic(Characteristic.Model, model);
        // Add listener to identify event
        this.on('identify', (paired, callback) => {
            if (paired)
                logger.info('IDENTIFY device ' + this.eui64);
            callback();
        });
        this.addEndpoint(endpoint, type);
    }

    getEndpoint(endpoint) {
        let endpointName = this.eui64.substr(this.eui64.length - 6) + '-' + endpoint;
        for (let i in this.endpoints) {
            if (this.endpoints[i].name === endpointName) {
                return this.endpoints[i];
            }
        }
    }

    getEndpointName(endpoint) {
        return this.eui64.substr(this.eui64.length - 6) + '-' + endpoint;
    }

    addInformation(serialNumber, manufacturer, model) {
        this.getService(Service.AccessoryInformation)
            .setCharacteristic(Characteristic.SerialNumber, serialNumber)
            .setCharacteristic(Characteristic.Manufacturer, manufacturer)
            .setCharacteristic(Characteristic.Model, model)
    }

    addEndpoint(endpoint, type) {
        if (this.getEndpoint(endpoint) !== undefined) {
            return;
        }
        logger.info('ADD endpoint ' + this.getEndpointName(endpoint));
        switch (type) {
            case EndpointType.SWITCH: {
                let newEndpoint = new Endpoint.SwitchEndpoint(this.eui64, endpoint);
                this.addService(newEndpoint);
                this.endpoints.push(newEndpoint);
            } break;
            case EndpointType.LIGHT: {
                let newEndpoint = new Endpoint.LightEndpoint(this.eui64, endpoint);
                this.addService(newEndpoint);
                this.endpoints.push(newEndpoint);
            } break;
            case EndpointType.CONTACT_SENSOR: {
                let newEndpoint = new Endpoint.ContactSensorEndpoint(this.eui64, endpoint);
                this.addService(newEndpoint);
                this.endpoints.push(newEndpoint);
            } break;
            case EndpointType.MOTION_SENSOR: {
                let newEndpoint = new Endpoint.MotionSensorEndpoint(this.eui64, endpoint);
                this.addService(newEndpoint);
                this.endpoints.push(newEndpoint);
            } break;
            case EndpointType.LIGHT_SENSOR: {
                let newEndpoint = new Endpoint.LightSensorEndpoint(this.eui64, endpoint);
                this.addService(newEndpoint);
                this.endpoints.push(newEndpoint);
            } break;
            case EndpointType.TEMPERATURE_SENSOR: {
                let newEndpoint = new Endpoint.TemperatureSensorEndpoint(this.eui64, endpoint);
                this.addService(newEndpoint);
                this.endpoints.push(newEndpoint);
            } break;
            case EndpointType.HUMIDITY_SENSOR: {
                let newEndpoint = new Endpoint.HumiditySensorEndpoint(this.eui64, endpoint);
                this.addService(newEndpoint);
                this.endpoints.push(newEndpoint);
            } break;
            case EndpointType.BATTERY: {
                let newEndpoint = new Endpoint.BatteryEndpoint(this.eui64, endpoint);
                this.addService(newEndpoint);
                this.endpoints.push(newEndpoint);
            } break;
            default: break;
        }
    }

    removeEndpoint(endpoint) {
        logger.info('REMOVE endpoint ' + this.getEndpointName(endpoint));
        this.removeService(this.getEndpointName(endpoint));
        for (let i in this.endpoints) {
            if (this.endpoints[i] === this.getEndpointName(endpoint)) {
                this.endpoints.splice(i, 1);
                break;
            }
        }
    }

    updateEndpointValue(value, endpoint) {
        if (this.getEndpoint(endpoint) !== undefined) {
            switch (this.getEndpoint(endpoint).type) {
                case EndpointType.SWITCH: {
                    if (value.on !== undefined) {
                        this.getEndpoint(endpoint).updateValue(value.on);
                    }
                } break;
                case EndpointType.LIGHT: {
                    if (value.on !== undefined || value.brightness !== undefined) {
                        this.getEndpoint(endpoint).updateValue(value);
                    }
                } break;
                case EndpointType.CONTACT_SENSOR: {
                    if (value.contact !== undefined) {
                        this.getEndpoint(endpoint).updateValue(value.contact);
                    }
                } break;
                case EndpointType.MOTION_SENSOR: {
                    if (value.motion !== undefined) {
                        this.getEndpoint(endpoint).updateValue(value.motion);
                    }
                } break;
                case EndpointType.LIGHT_SENSOR: {
                    if (value.lux !== undefined) {
                        this.getEndpoint(endpoint).updateValue(value.lux);
                    }
                } break;
                case EndpointType.TEMPERATURE_SENSOR: {
                    if (value.temperature !== undefined) {
                        this.getEndpoint(endpoint).updateValue(value.temperature);
                    }
                } break;
                case EndpointType.HUMIDITY_SENSOR: {
                    if (value.humidity !== undefined) {
                        this.getEndpoint(endpoint).updateValue(value.humidity);
                    }
                } break;
                case EndpointType.BATTERY: {
                    if (value.battery !== undefined) {
                        this.getEndpoint(endpoint).updateValue(value.battery);
                    }
                } break;
                default: break;
            }
        }
    }

    setEndpointValue(value, endpoint) {
        if (value.on !== undefined || value.level !== undefined)
                if (this.getEndpoint(endpoint) !== undefined)
                    this.getEndpoint(endpoint).setValue(value);
    }
}

module.exports = { Device, DeviceType };