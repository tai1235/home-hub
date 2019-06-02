/**
 * Author: TaiNV
 * Date Created: 2019/04/15
 * Module: devices-command
 * Description: Device messages implementation
 */

let { Command, StatusCode } = require('../command');

/**
 * Requests received
 * Responses are required
 */

const CommandSearch = {
    parseData: data => {
        // Validate data fields
        let act = data.act !== undefined &&
            typeof data.act === 'number' &&
            [0, 1].indexOf(data.act)  !== -1
            ? data.act : -1;

        if (act !== -1) {
            return {
                result: StatusCode.SUCCESS,
                data: { act }
            };
        } else {
            return {
                result: StatusCode.INVALID_DATA_CONTENT
            };
        }
    },

    createResponse: (statusCode, statusString) => {
        return new Command('devices', 'search', { statusCode, statusString, returnData: {} });
    },

    createStatus: (status, duration) => {
        return new Command('devices', 'search', { status, duration });
    }
};

const CommandConfig = {

};

const CommandRemove = {

};

/**
 * Requests received
 * Responses are not required
 */

const CommandWakeUp = {

};

const CommandControl = {
    parseData: data => {
        let devices = data.devices !== undefined &&
            data.devices instanceof Array &&
            data.devices.length > 0
            ? data.devices : false;
        if (devices) {
            for (let device of devices) {
                let eui64 = device.eui64 !== undefined &&
                typeof device.eui64 === 'string' &&
                device.eui64.trim().length > 0
                    ? device.eui64 : false;
                let endpoint = device.endpoint !== undefined &&
                    typeof device.endpoint === 'number' &&
                    device.endpoint > 0
                    ? device.endpoint : -1;
                let value = device.value !== undefined &&
                    typeof device.value === 'object'
                    ? device.value : false;
                if (!eui64 || endpoint === -1 || !value) {
                    return {
                        result: StatusCode.INVALID_DATA_CONTENT
                    }
                }
            }
            return {
                result: StatusCode.SUCCESS,
                data: devices
            }
        } else {
            return {
                result: StatusCode.INVALID_DATA_CONTENT
            }
        }
    },

    createResponse: (statusCode, statusString) => {
        return new Command('devices', 'control', { statusCode, statusString, returnData: {} });
    },
};

/**
 * Requests to send
 * Responses are required
 */

const CommandAdd = {
    createRequest: (version, devices) => {
        let parsedVersion = version !== undefined &&
            typeof version === 'number'
            ? version : -1;
        let parsedDevices = devices !== undefined &&
            devices instanceof Array &&
            devices.length > 0
            ? devices : false;
        if (parsedVersion !== -1 && parsedDevices) {
            return {
                result: StatusCode.SUCCESS,
                data: new Command('devices', 'add', {
                    version: parsedVersion,
                    devices: parsedDevices
                })
            }
        } else {
            return {
                result: StatusCode.INVALID_DATA_CONTENT
            }
        }
    }
};

/**
 * Requests to send
 * Responses are not required
 */

const CommandOnline = {
    createRequest: (devices) => {
        return new Command('devices', 'online', { devices });
    }
};

const CommandStatus = {
    createStatus: (eui64, endpoint, value) => {
        return new Command('devices', 'status', { eui64, endpoint, value });
    }
};

/**
 * Both ways
 */

const CommandSync = {
    createRequest: () => {
        return new Command('devices', 'sync', {});
    },

    createResponse: (statusCode, statusString, devices) => {
        return new Command('devices', 'sync', { statusCode, statusString, returnData: { devices }});
    }
};

const CommandVersion = {
    createRequest: () => {
        return new Command('devices', 'version', {});
    },

    createResponse: (statusCode, statusString, version) => {
        return new Command('devices', 'version', { statusCode, statusString, returnData: { version }});
    }
};

module.exports = {
    CommandSearch,  // Done
    CommandAdd,     // Done
    CommandRemove,
    CommandControl, // Done
    CommandStatus,  // Done
    CommandConfig,
    CommandSync,    // Done
    CommandVersion, // Done
    CommandOnline, // Done
    CommandWakeUp
};