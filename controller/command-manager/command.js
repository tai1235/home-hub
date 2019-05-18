/**
 * Author: TaiNV
 * Date Created: 2019/04/15
 * Module: message-format
 * Description: Message format used by all messages
 */

let Logger = require('../../libraries/system-log');

let logger = new Logger(__filename);

const StatusCode = {
    SUCCESS: 0,
    ACTION_FAILED: 1,
    ACTION_NOT_SUPPORTED: 2,
    INVALID_JSON_FORMAT: 3,
    INVALID_DATA_CONTENT: 4
};

class Command {
    constructor(type, action, data) {
        this.type = type;
        this.action = action;
        this.data = data
    }

    get fullCommand() {
        return JSON.stringify({
            type: this.type,
            action: this.action,
            data: this.data
        })
    }

    static parseCommand(message) {
        try {
            // Parse message object
            let parsedMessage = JSON.parse(message);
            // Validate object properties
            let type = parsedMessage.type !== undefined &&
                typeof parsedMessage.type === 'string' &&
                ['devices', 'rules', 'groups', 'errors'].indexOf(parsedMessage.type) > -1 ?
                parsedMessage.type : false;
            let action = parsedMessage.action !== undefined &&
                typeof parsedMessage.action === 'string' ?
                parsedMessage.action : false;
            let data = parsedMessage.data !== undefined &&
                typeof parsedMessage.data === 'object' ?
                parsedMessage.data : false;
            // Return
            if (type && action && data) {
                return {
                    result: StatusCode.SUCCESS,
                    command: new Command(type, action, data)
                }
            } else {
                return {
                    result: StatusCode.INVALID_DATA_CONTENT
                };
            }
        } catch (e) {
            logger.error(e.message);
            return {
                result: StatusCode.INVALID_JSON_FORMAT
            }
        }

    }
}

module.exports = { Command, StatusCode };