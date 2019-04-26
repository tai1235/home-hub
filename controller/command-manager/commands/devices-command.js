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

class CommandControl {

}

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
            ? devices : [];
        if (parsedVersion !== -1 && parsedDevices !== []) {
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

const CommandOffline = {

};

const CommandStatus = {

};

/**
 * Both ways
 */

const CommandSync = {

};

const CommandVersion = {

};

module.exports = {
    CommandSearch,
    CommandAdd,
    CommandRemove,
    CommandControl,
    CommandStatus,
    CommandConfig,
    CommandSync,
    CommandVersion,
    CommandOffline,
    CommandWakeUp
};