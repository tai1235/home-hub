/**
 * Author: TaiNV
 * Date Created: 2019/02/17
 * Module: db-manager
 * Description: Device schema definition
 */

// Dependencies
let mongoose = require('mongoose');
let Logger = require('../../libraries/system-log');

let logger = new Logger(__filename);

let DeviceSchema = new mongoose.Schema({
    _id: mongoose.Schema.Types.ObjectId,
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
    endpoints: [{
        endpoint: {
            type: Number,
            required: true
        },
        type: {
            type: String,
            required: true,
            trim: true
        }
    }],
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
    addDevice: (eui64, endpoint, type) => {
        logger.debug({eui64, endpoint, type});
        Devices.find({ eui64: eui64 })
            .exec()
            .then(device => {
                if (device.length === 0) {
                    let newDevice = new Devices({
                        _id: new mongoose.Types.ObjectId,
                        eui64: eui64,
                        endpoints: [{
                            endpoint, type
                        }]
                    });
                    newDevice
                        .save()
                        .then(device => {
                            logger.debug(device);
                            logger.info('ADD device ' + eui64);
                        })
                        .catch(err => logger.error(err.message));
                } else {
                    logger.error('Device already existed in database');
                }
            })
            .catch();
    },

    updateDeviceInfo: (eui64, params) => {
        // Find and update device
        Devices.findOne({ eui64: eui64})
            .exec()
            .then(device => {
                // Return if no device found
                if (!device) {
                    logger.error('Could not find device to update information');
                    return;
                }
                // Validate params and update device
                if (params.name)
                    device.information.name = params.name;
                if (params.manufacturer)
                    device.information.manufacturer = params.manufacturer;
                if (params.model)
                    device.information.model = params.model;
                if (params.serialNumber)
                    device.information.serialNumber = params.serialNumber;
                // Save changes
                device
                    .save()
                    .then(device => {
                        logger.debug(device);
                        logger.info('UPDATE device ' + eui64);
                    })
                    .catch(err => logger.error(err.message));
            })
            .catch()
    },

    addEndpoint: (eui64, endpoint, type) => {
        Devices.findOne({ eui64: eui64})
            .exec()
            .then(device => {
                // Return if no device found
                if (device.length === 0) {
                    logger.error('Could not found device to add endpoint');
                    return;
                }
                // Return if endpoint existed
                for (let i in device.endpoints) {
                    if (endpoint === device.endpoints[i].endpoint) {
                        logger.warn('Endpoint already existed');
                        return;
                    }
                }
                // Update device's endpoint
                device.endpoints.push({
                    endpoint, type
                });
                // Save changes
                device
                    .save()
                    .then(device => {
                        logger.debug(device);
                        logger.info('ADD endpoint ' + eui64 + '_' + endpoint);
                    })
                    .catch(err => logger.error(err.message));
            })
            .catch(err => logger.error(err.message));
    },

    removeDevice: eui64 => {
        Devices.remove({ eui64: eui64})
            .exec()
            .then(result => {
                if (result.n > 0 && result.ok === 1)
                    logger.info('REMOVE device ' + eui64);
            })
            .catch(err => logger.error(err.message));
    },

    getAllDevices: callback => {
        Devices.find({})
            .exec()
            .then(devices => callback(devices))
            .catch(err => logger.error(err.message));
    },

    handleDeviceJoined: (eui64, endpoint, type) => {
        Devices.findOne({ eui64: eui64 })
            .exec()
            .then(device => {
                if (device) {
                    let deviceExisted = false;
                    for (let i in device.endpoints) {
                        if (endpoint === device.endpoints[i].endpoint) {
                            logger.warn('Device and endpoint already existed');
                            deviceExisted = true;
                        }
                    }
                    if (deviceExisted) {
                        device.online = true;
                        device
                            .save()
                            .then(device => {
                                logger.debug(device);
                                logger.info('ONLINE device ' + eui64);
                            })
                            .catch(err => logger.error(err.message));
                    } else {
                        DevicesDB.addEndpoint(eui64, endpoint, type);
                    }
                } else {
                    DevicesDB.addDevice(eui64, endpoint, type)
                }
            })
            .catch(err => logger.error(err.message));
    },

    handleDeviceLeft: eui64 => {
        Devices.findOne({ eui64: eui64})
            .exec()
            .then(device => {
                // Return if no device found
                if (!device) {
                    logger.warn('Device has not stored in database');
                    return;
                }
                // Update online status of device
                device.online = false;
                // Save changes
                device
                    .save()
                    .then(device => {
                        logger.debug(device);
                        logger.info('OFFLINE device ' + eui64);
                    })
                    .catch()
            })
            .catch()
    },

    handleDeviceRemove: eui64 => DevicesDB.removeDevice(eui64),
    handleDeviceUpdate: (eui64, params) => DevicesDB.updateDeviceInfo(eui64, params)
};

module.exports = DevicesDB;

