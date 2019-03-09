/**
 * Author: TaiNV
 * Date Created: 2019/02/17
 * Module: device-manager
 * Description: provide management for all device connected to this hub
 */

// Dependencies
let Bridge = require('../../hap').Bridge
let uuid = require('../../hap').uuid
let Logger = require('../../libraries/system-log')

let logger = new Logger(__filename)

class DeviceManager extends Bridge {
    constructor() {
        this.devices = []
        this.bridge = new Bridge('Home Hub', uuid.generate('Home Hub'));
        this.on('identify', (paired, callback) => {
            if (paired)
                logger.info('Bridge has been identified')
            callback();
        })
        this.bridgeConfig = {
            username: "9C:23:12:45:F2:A3",
            port: 12674,
            pincode: "031-45-154",
            category: Accessory.Categories.BRIDGE
        }
    }

    start() {
        this.bridge.publish(this.bridgeConfig)
    }

    addDevice(deviceToAdd) {
        let deviceExisted = false;
        this.devices.forEach(device => {
            if (device.eui64 == deviceToAdd.eui64) {
                logger.debug('This device is existed');
                deviceExisted = true
                break
            }
        })
        if (!deviceExisted) {
            this.devices.push(deviceToAdd)
            this.bridge.addBridgedAccessory(deviceToAdd.accessory)
        }
    }

    removeDevice(deviceToRemove) {
        for (i in this.devices) {
            if (deviceToRemove.eui64 == this.devices[i].eui64) {
                this.bridge.removeremoveBridgedAccessories(this.devices[i].accessory)
                this.devices.splice(i, 1)
            }
        }
    }
}

module.exports = DeviceManager