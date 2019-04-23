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
        let mac = data.mac !== undefined &&
            typeof data.mac === 'string' &&
            data.mac.trim().length > 0
            ? data.mac : false;
        let act = data.act !== undefined &&
            typeof data.act === 'number' &&
            [0, 1].indexOf(data.act) > -1
            ? data.act : false;

        if (mac && act) {
            return {
                result: StatusCode.SUCCESS,
                data: { mac, act }
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

    createStatus: (mac, status, duration) => {
        return new Command('devices', 'search', { mac, status, duration });
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