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
let { SwitchEndpoint, LightEndpoint } = require('../../controller/device-manager/endpoint');
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
        let endpointName = this.eui64 + '_' + endpoint;
        for (let i in this.endpoints) {
            if (this.endpoints[i].name === endpointName) {
                return this.endpoints[i];
            }
        }
    }

    getEndpointName(endpoint) {
        return this.eui64 + '_' + endpoint;
    }

    addInformation(serialNumber, manufacturer, model) {
        this.getService(Service.AccessoryInformation)
            .setCharacteristic(Characteristic.SerialNumber, serialNumber)
            .setCharacteristic(Characteristic.Manufacturer, manufacturer)
            .setCharacteristic(Characteristic.Model, model)
    }

    addEndpoint(endpoint, type) {
        logger.info('ADD endpoint ' + this.getEndpointName(endpoint));
        switch (type) {
            case EndpointType.SWITCH: {
                let newEndpoint = new SwitchEndpoint(this.eui64, endpoint);
                this.addService(newEndpoint);
                this.endpoints.push(newEndpoint);
            } break;
            case EndpointType.LIGHT: {
                let newEndpoint = new LightEndpoint(this.eui64, endpoint);
                this.addService(newEndpoint);
                this.endpoints.push(newEndpoint);
            } break;
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
        if (value.on === undefined || value.level === undefined)
            return;
        this.getEndpoint(endpoint).updateValue(value);
    }
}

module.exports = { Device, DeviceType };