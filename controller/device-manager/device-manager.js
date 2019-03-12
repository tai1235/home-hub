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

let logger = new Logger(__filename);

class DeviceManager extends Bridge {
    constructor() {
        super('Home Hub', uuid.generate('Home Hub'));
        this.devices = [];
        this.currentEui64 = '';
        this.currentEndpoint = 0;
        this.on('identify', (paired, callback) => {
            if (paired)
                logger.info('Bridge has been identified');
            callback();
        });
        this.bridgeConfig = {
            username: "9C:53:12:45:F2:C3",
            port: 56423,
            pincode: "031-45-154",
            category: Accessory.Categories.BRIDGE
        }
    }

    start() {
        this.publish(this.bridgeConfig);
    }

    addDevice(deviceToAdd) {
        let deviceExisted = false;
        for (let i in this.devices) {
            if (this.devices[i].eui64 === deviceToAdd.eui64) {
                logger.debug('This device is existed');
                deviceExisted = true;
            }
        }
        if (!deviceExisted) {
            // Adding new device
            logger.info("Adding new device: " + eui64);
            this.devices.push(deviceToAdd);
            this.addBridgedAccessory(deviceToAdd);
        } else {
            // Update device
        }
    }

    getDevice(eui64) {
        for (let i in this.devices) {
            if (this.devices[i].eui64 === eui64) {
                return this.devices[i];
            }
        }
    }

    removeDevice(eui64) {
        for (let i in this.bridgedAccessories) {
            if (eui64 === this.devices[i].eui64) {
                this.removeBridgedAccessories(this.devices[i]);
                this.devices.splice(i, 1);
            }
        }
    }

    handleDeviceJoined(eui64, endpoint, type) {
        if (this.currentEui64 === eui64) {
            if (this.currentEndpoint === endpoint) {
                logger.info("This endpoint has been added");
            } else {
                this.getDevice(eui64).addEndpoint(endpoint, type);
                this.currentEndpoint = endpoint;
            }
        } else {
            let newDevice = new Device(eui64, endpoint, type);
            this.addDevice(newDevice);
            this.currentEui64 = eui64;
        }
    }

    handleDeviceLeft(eui64) {
        this.removeDevice(eui64);
    }
}

module.exports = DeviceManager;