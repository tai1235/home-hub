/**
 * Author: TaiNV
 * Date Created: 2019/02/17
 * Module: database-manager
 * Description: Device schema definition
 */

// Dependencies
let mongoose = require('mongoose');
let Logger = require('../../libraries/system-log');

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
            trim: true,
            default: 'TTC'
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

const Devices = new mongoose.model('Devices', DeviceSchema);

const DevicesDB = {
    getAllDevices: callback => {
        Devices.find({})
            .exec()
            .then(devices => callback(devices))
            .catch(e => logger.error(e.message));
    },
    handleDeviceJoined: (eui64, endpoint, type) => {
        Devices.find({ eui64: eui64 })
            .exec()
            .then(devices => {
                if (devices === null) {
                    logger.info('ADD endpoint ' + eui64 + '_' + endpoint);
                    let newDevice = new Devices({ eui64, endpoint, type });
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
                    let newDevice = new Devices({ eui64, endpoint, type });
                    return newDevice.save();
                }
            })
            .then(device => {
                if (device) {
                    logger.debug(device);
                    logger.info('UPDATE database successfully')
                }
            })
            .catch(e => logger.error(e.message));
    },
    handleDeviceLeft: eui64 => {
        Devices.find({ eui64: eui64 })
            .exec()
            .then(devices => {
                for (let device of devices) {
                    device.online = false;
                }
                return Devices.create(devices);
            })
            .then(devices => {
                logger.debug(devices);
                logger.info('OFFLINE device ' + eui64);
            })
            .catch()
    },
    handleDeviceRemove: eui64 => {
        Devices.remove({ eui64: eui64 })
            .exec()
            .then(result => {
                if (result.n > 0 && result.ok)
                    logger.info('REMOVE device ' + eui64);
            })
            .catch(e => logger.error(e.message));
    },
    handleDeviceStatus: eui64 => {
        Devices.find({ eui64: eui64 })
            .exec()
            .then(devices => {
                for (let device of devices) {
                    if (!device.online)
                        device.online = true;
                }
                return Devices.create(devices);
            })
            .then(devices => {
                logger.debug(devices);
                logger.info('ONLINE device ' + eui64);
            })
            .catch(e => logger.error(e.message));
    }
};

module.exports = DevicesDB;

