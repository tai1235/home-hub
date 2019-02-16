/**
 * Author: TaiNV
 * Date Modified: 2019/01/19
 * Module: system-log
 * Description: provie logging interface for the project
 */

// Dependencies
const {createLogger, format, transports} = require('winston');
const moment = require('moment');
const path = require('path');
const config = require('../../config');

// Container for this module
const Log = {};

Log.config = {
    filename: '',
    logfile: config.logPath
}

// Create Winston logger
const logger = createLogger({
    level: 'debug',
    transports: [
        new transports.Console({
            format: format.printf(msg => format.colorize().colorize(msg.level, 
                    '[' + moment().format("YYYY-MM-DD HH:mm:ss.SSS") + ']' +
                    '[' + msg.level.toUpperCase() + ']' +
                    '[' + Log.config.filename + '] ' +
                    msg.message))
        }),
        new transports.File({
            filename: Log.config.logfile,
            level: 'info',
            format: format.printf(msg => (
                    '[' + moment().format("YYYY-MM-DD HH:mm:ss.SSS") + ']' +
                    '[' + msg.level.toUpperCase() + ']' +
                    '[' + Log.config.filename + '] ' +
                    msg.message))
        })
    ]
});

// Create log with filename for debug purposes
Log.create = filename => {
    filename = typeof(filename) == 'string' ? filename : false;
    Log.config.filename = path.parse(filename).base;
    return Log;
}

// Log debug
Log.debug = message => {
    logger.debug(message);
}

// Log info
Log.info = message => {
    logger.info(message);
}

// Log warn
Log.warn = message => {
    logger.warn(message);
}

// Log error
Log.error = message => {
    logger.error(message);
}


// Export the module
module.exports = Log;