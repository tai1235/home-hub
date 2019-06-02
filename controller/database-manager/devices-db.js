/**
 * Author: TaiNV
 * Date Created: 2019/02/17
 * Module: database-manager
 * Description: Device schema definition
 */

// Dependencies
let mongoose = require('mongoose');
let Logger = require('../../libraries/system-log');
let Versions = require('./versions');

let logger = new Logger(__filename);

let DeviceSchema = new mongoose.Schema({
    eui64: {
        type: String,
        required: true
    },
    information: {
        name: {
            type: String,
            trim: true
        },
        serialNumber: {
            type: String,
            uppercase: true,
            trim: true,
        },
        manufacturer: {
            type: String,
            trim: true
        },
        model: {
            type: String,
            trim: true,
            default: '1.0'
        }
    },
    endpoint: {
        type: Number,
        required: true
    },
    type: {
        type: String,
        required: true,
        trim: true
    },
    room: {
        type: String,
        trim: true
    },
    group: {
        type: String,
        trim: true
    },
    online: {
        type: Boolean,
        required: true,
        default: true
    }
});

class Devices  {
    constructor() {
        this.devices = new mongoose.model('Devices', DeviceSchema);
    }

    getAllDevices(callback) {
        this.devices.find({})
            .exec()
            .then(devices => callback(false, devices))
            .catch(e => {
                logger.error('GetAllDevices: ' + e.message);
                callback(e)
            });
    }

    handleDeviceJoined(eui64, endpoint, type, callback) {
        let deviceUpdate = true;
        this.devices.find({ eui64: eui64 })
            .exec()
            .then(devices => {
                if (devices === null) {
                    logger.info('ADD endpoint ' + eui64 + '_' + endpoint);
                    let newDevice = new this.devices({ eui64, endpoint, type });
                    deviceUpdate = false;
                    return newDevice.save();
                } else {
                    for (let device of devices) {
                        if (device.endpoint === endpoint) {
                            if (!device.online) {
                                logger.info('ONLINE device ' + eui64);
                                device.online = true;
                                return device.save();
                            } else {
                                return;
                            }
                        }
                    }
                    logger.info('ADD endpoint ' + eui64 + '_' + endpoint);
                    let newDevice = new this.devices({ eui64, endpoint, type });
                    deviceUpdate = false;
                    return newDevice.save();
                }
            })
            .then(device => {
                if (device) {
                    logger.debug(device);
                    logger.info('UPDATE database successfully');
                    Versions.update('devices');
                    let version = Versions.get('devices');
                    callback(false, {
                        version,
                        updated: deviceUpdate,
                        data: [ device ] });
                }
            })
            .catch(e => {
                logger.error('HandleDeviceJoined: ' + e.message);
                callback(e);
            });
    }

    handleDeviceLeft(eui64, callback) {
        this.devices.find({ eui64: eui64 })
            .exec()
            .then(devices => {
                for (let device of devices) {
                    device.online = false;
                }
                return this.devices.create(devices);
            })
            .then(devices => {
                if (devices) {
                    logger.debug(devices);
                    logger.info('OFFLINE device ' + eui64);
                    Versions.update('devices');
                    let version = Versions.get('devices');
                    callback(false, { version, data: devices });
                }
            })
            .catch(e => {
                logger.error('HandleDeviceLeft: ' + e.message);
                callback(e);
            });
    }

    handleDeviceRemove(eui64, callback) {
        this.devices.remove({ eui64: eui64 })
            .exec()
            .then(result => {
                if (result.n > 0 && result.ok) {
                    logger.info('REMOVE device ' + eui64);
                    Versions.update('devices');
                    let version = Versions.get('devices');
                    callback(false, version);
                }
            })
            .catch(e => {
                logger.error('HandleDeviceRemove: ' + e.message);
                callback(e);
            });
    }

    handleDeviceStatus(eui64, callback) {
        this.devices.find({ eui64: eui64 })
            .exec()
            .then(devices => {
                let updated = false;
                for (let device of devices) {
                    if (!device.online) {
                        device.online = true;
                        updated = true;
                    }
                }
                if (updated)
                    return this.devices.create(devices);
            })
            .then(devices => {
                if (devices) {
                    logger.info('ONLINE device ' + eui64);
                    Versions.update('devices');
                    let version = Versions.get('devices');
                    callback(false, { version, data: devices });
                }
            })
            .catch(e => {
                logger.error('HandleDeviceStatus: ' + e.message);
                callback(e);
            });
    }
}

module.exports = Devices;

