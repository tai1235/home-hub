/**
 * Author: TaiNV
 * Date Created: 2019/01/19
 * Module: system-log
 * Description: provie logging interface for the project
 */

// Dependencies
const { createLogger, format, transports } = require('winston');
const moment = require('moment');
const path = require('path');
const config = require('./config');

class Logger {
    constructor(filename) {
        this.filename = path.basename(filename)
        this.logger = createLogger({
            level: 'debug',
            transports: [
                new transports.Console({
                    format: format.printf(msg => format.colorize().colorize(msg.level,
                        '[' + moment().format("YYYY-MM-DD HH:mm:ss.SSS") + ']' +
                        '[' + this.filename + '] ' +
                        msg.message))
                }),
                new transports.File({
                    filename: config.logPath,
                    level: 'info',
                    format: format.printf(msg => (
                        '[' + moment().format("YYYY-MM-DD HH:mm:ss.SSS") + ']' +
                        '[' + msg.level.substring(0, 4).toUpperCase() + ']' +
                        '[' + this.filename + '] ' +
                        msg.message))
                })
            ]
        })
    }

    debug(message) {
        this.logger.debug(message)
    }

    info(message) {
        this.logger.info(message)
    }

    warn(message) {
        this.logger.warn(message)
    }

    error(message) {
        this.logger.error(message)
    }
}

// Export the module
module.exports = Logger;