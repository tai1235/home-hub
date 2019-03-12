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

let logger = new Logger(__filename);

const DeviceType = {
    SWITCH: 'switch',
    LIGHT: 'light',
    CURTAIN: 'curtain',
    SENSOR: 'sensor'
};

// Device class
class Device extends Accessory {
    constructor(eui64, name, type, serialNumber = '', manufacturer = '', model = '') {
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
    }

    addEndpoint(endpoint, type) {
        logger.info("Adding endpoint %d to device %s", endpoint, this.eui64);
        switch (type) {
            case DeviceType.SWITCH: {
                let newEndpoint = new SwitchEndpoint(this.eui64, endpoint);
                this.addService(newEndpoint);
                this.endpoints.push(newEndpoint.name);
            } break;
            case DeviceType.LIGHT: {
                let newEndpoint = new LightEndpoint(this.eui64, endpoint);
                this.addService(newEndpoint);
                this.endpoints.push(newEndpoint.name);
            } break;
        }
    }
}

module.exports = { Device, DeviceType };