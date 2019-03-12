/**
 * Author: TaiNV
 * Date Created: 2019/02/17
 * Module: db-manager
 * Description: Device schema definition
 */

// Dependencies
let mongoose = require('mongoose');
let DeviceType = require('../device-manager/device').DeviceType;

let ServiceSchema = new mongoose.Schema({
    id: {
        type: String,
        required: true
    },
    characteristics: [{
        id: {
            type: String,
            required: true
        }
    }]
});

let DeviceSchema = new mongoose.Schema({
    eui64: {
        type: String,
        required: true
    },
    information: {
        name: {
            type: String,
            trim: true,
            required: true
        },
        serialNumber: {
            type: String,
            lowercase: true,
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
    services: [ServiceSchema],
    room: {
        type: String,
        trim: true,
        default: ''
    },
    group: {
        type: String,
        trim: true,
        default: ''
    }
});

class DevicesDB {
    constructor() {
        this.schema = mongoose.model('Device', DeviceSchema);
    }

    addDevice(eui64, name, type, endpoint, serialNumber = '', manufacturer = '', model = '') {
        let deviceToStore = {};
        deviceToStore.eui64 = eui64;
        deviceToStore.information = {};
        deviceToStore.information.name = name
        if (serialNumber)
            deviceToStore.information.serialNumber = device.serialNumber;
        if (manufacturer)
            deviceToStore.information.manufacturer = device.manufacturer;
        if (model)
            deviceToStore.information.model = device.model;
        if (type === DeviceType.SWITCH) {
            deviceToStore.services.push({
                id: eui64 + '_' + endpoint,
                characteristics: [].push(eui64 + '_' + endpoint + '_on')
            });
        }
        this.schema.add(deviceToStore);
    }

    removeDevice(eui64) {

    }

    updateDevice(eui64, ) {

    }

    getDevice(eui64) {

    }
}

module.exports = DevicesDB;

