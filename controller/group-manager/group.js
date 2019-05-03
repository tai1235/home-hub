/**
 * Author: TaiNV
 * Date Modified: 2019/03/31
 * Module: group
 * Description: group's implementation
 */

// Dependencies
let helpers = require('../../libraries/helpers');

class Group {
    constructor(groupid = '') {
        this.id = groupid === '' ? helpers.createRandomString(32) : id;
        this.devices = [];
        this.status = {};
        this.deviceControl = [];
        this.deviceStatus = [];
    }

    addDevices(devices) {
        let deviceType = devices[0].type;
        if (devices.every(device => device.type === deviceType)) {
            this.devices = devices;
        } else {
            logger.warn('Could not add devices with different type to group');
        }
    }

    handleDeviceStatus(eui64, endpoint, value) {

    }
}