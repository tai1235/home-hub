var fmt = require('util').format;
var chalk = require('chalk');
var EventEmitter = require('events').EventEmitter;

var Log = exports = module.exports = function Log(stream) {
    this.stream = stream || process.stdout;
};

level = {
    FATAL: { id: 0, prefix: chalk.blue('[FATAL]') },
    CRITICAL: { id: 1, prefix: chalk.red('[CRIT ]') },
    ERROR: { id: 2, prefix: chalk.red('[ERROR]') },
    WARNING: { id: 3, prefix: chalk.yellow('[WARN ]') },
    INFORMATION: { id: 4, prefix: chalk.green('[INFO ]') },
    DEBUG: { id: 5, prefix: chalk.white('[DEBUG]') },
}

Log.prototype = {
    log: function (level, args) {
        var msg = fmt.apply(null, args);
        this.stream.write(
            '[' + new Date + ']'
            + level.prefix
            + ' ' + msg
            + '\n'
        );
    },

    fatal: function (msg) {
        this.log(level.FATAL, arguments);
    },

    critical: function (msg) {
        this.log(level.CRITICAL, arguments);
    },

    error: function (msg) {
        this.log(level.ERROR, arguments);
    },

    warning: function (msg) {
        this.log(level.WARNING, arguments);
    },

    info: function (msg) {
        this.log(level.INFORMATION, arguments);
    },

    debug: function (msg) {
        this.log(level.DEBUG, arguments);
    }
};

Log.prototype.__proto__ = EventEmitter.prototype;
