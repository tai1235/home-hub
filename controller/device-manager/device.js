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
        super(name, uuid.generate('hap-nodejs:accessories:' + type + name));
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
                logger.debug('Device ' + this.eui64 + ' has been identified');
            callback();
        });
        this.addEndpoint(endpoint, type);
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
        logger.info("Adding endpoint " + endpoint + " to device " + this.eui64);
        switch (type) {
            case EndpointType.SWITCH: {
                let newEndpoint = new SwitchEndpoint(this.eui64, endpoint);
                this.addService(newEndpoint);
                this.endpoints.push(newEndpoint.name);
            } break;
            case EndpointType.LIGHT: {
                let newEndpoint = new LightEndpoint(this.eui64, endpoint);
                this.addService(newEndpoint);
                this.endpoints.push(newEndpoint.name);
            } break;
        }
    }

    removeEndpoint(endpoint) {
        logger.info("Removing endpoint " + endpoint + " from device " + this.eui64);
        this.removeService(this.getEndpointName(endpoint));
        for (let i in this.endpoints) {
            if (this.endpoints[i] === this.getEndpointName(endpoint)) {
                this.endpoints.splice(i, 1);
                break;
            }
        }
    }

    updateValue(value, endpoint) {
        this.getService(this.getEndpointName(endpoint))
            .updateValue(value);
    }
}

module.exports = { Device, DeviceType };