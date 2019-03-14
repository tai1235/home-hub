/**
 * Author: TaiNV
 * Date Created: 2019/02/17
 * Module: device-manager
 * Description: Provide management for all device connected to this hub
 */

// Dependencies
let Accessory = require('../../hap').Accessory;
let Bridge = require('../../hap').Bridge;
let uuid = require('../../hap').uuid;
let Logger = require('../../libraries/system-log');
let Device = require('../../controller/device-manager/device').Device;

let logger = new Logger(__filename);

class DeviceManager extends Bridge {
    constructor() {
        super('Home Hub', uuid.generate('Home Hub'));
        this.devices = [];
        this.on('identify', (paired, callback) => {
            if (paired)
                logger.info('Bridge has been identified');
            callback();
        });
        this.bridgeConfig = {
            username: '9C:13:12:45:F2:C3',
            port: 56423,
            pincode: '031-45-154',
            category: Accessory.Categories.BRIDGE
        }
    }

    start() {
        this.publish(this.bridgeConfig);
    }

    addDevice(deviceToAdd) {
        logger.info('ADD device: ' + deviceToAdd.eui64 + ' to bridge');
        this.devices.push(deviceToAdd);
        this.addBridgedAccessory(deviceToAdd);
    }

    getDevice(eui64) {
        for (let i in this.devices) {
            if (this.devices[i].eui64 === eui64) {
                return this.devices[i];
            }
        }
    }

    loadDeviceFromDB(deviceInfo) {
        // TODO
    }

    removeDevice(eui64) {
        logger.info('REMOVE device: ' + deviceToAdd.eui64 + ' from bridge');
        for (let i in this.bridgedAccessories) {
            if (eui64 === this.devices[i].eui64) {
                this.removeBridgedAccessories(this.devices[i]);
                this.devices.splice(i, 1);
            }
        }
    }

    handleDeviceJoined(eui64, endpoint, type) {
        if (this.getDevice(eui64) === undefined) {
            let newDevice = new Device(eui64, endpoint, type);
            this.addDevice(newDevice);
        } else {
            this.getDevice(eui64).addEndpoint(endpoint, type);
        }
    }

    handleDeviceLeft(eui64) {
        this.removeDevice(eui64);
    }

    handleDeviceStatus(value, eui64, endpoint) {
        this.getDevice(eui64).updateEndpointValue(value, endpoint);
    }
}

module.exports = DeviceManager;