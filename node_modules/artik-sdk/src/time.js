var events = require('events');
var util = require('util');
var time = require("../build/Release/artik-sdk.node").time;

var Time = function() {
    events.EventEmitter.call(this);
    this.time = time();
}

util.inherits(Time, events.EventEmitter);

module.exports = Time;

Time.prototype.get_time = function get_time() {
    return this.time.get_time();
}

Time.prototype.get_time_str = function get_time_str(format, timezone) {
    return this.time.get_time_str(format, timezone);
}

Time.prototype.set_time = function set_time(date) {
    return this.time.set_time(date);
}

Time.prototype.create_alarm_date = function create_alarm_date(timezone, date, func) {
    return this.time.create_alarm_date(timezone, date, func);
}

Time.prototype.create_alarm_second = function create_alarm_second(timezone, seconds, func) {
    return this.time.create_alarm_second(timezone, seconds, func);
}

Time.prototype.sync_ntp = function sync_ntp(hostname) {
    return this.time.sync_ntp(hostname);
}

Time.prototype.convert_timestamp_to_time = function convert_timestamp_to_time(timestamp) {
    return this.time.convert_timestamp_to_time(timestamp);
}

Time.prototype.convert_time_to_timestamp = function convert_time_to_timestamp(date) {
    return this.time.convert_time_to_timestamp(date);
}

module.exports.ARTIK_TIME = {
    'DFORMAT_DATE' : "h:m:s:S-d-D/M/Y\0",
    'ZONE' : {
        'GMT1' : 1,
        'GMT2' : 2,
        'GMT3' : 3,
        'GMT4' : 4,
        'GMT5' : 5,
        'GMT6' : 6,
        'GMT7' : 7,
        'GMT8' : 8,
        'GMT9' : 9,
        'GMT10' : 10,
        'GMT11' : 11,
        'GMT12' : 12
    }
};
