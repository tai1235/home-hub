var events = require('events');
var util = require('util');
var gpio = require('../build/Release/artik-sdk.node').gpio;

var Gpio = function(id, label, dir, edge, val) {
	events.EventEmitter.call(this);
	this.gpio = new gpio(id, label, dir, edge, val);
}

util.inherits(Gpio, events.EventEmitter);

module.exports = Gpio;

Gpio.prototype.request = function request() {
	var _ = this;
	return this.gpio.request(function(val) {
		_.emit('changed', val);
	});
};

Gpio.prototype.release = function release() {
	return this.gpio.release();
};

Gpio.prototype.read = function read() {
	return this.gpio.read();
};

Gpio.prototype.write = function write(val) {
	return this.gpio.write(val);
};

Gpio.prototype.get_name = function get_name() {
	return this.gpio.get_name();
};

Gpio.prototype.get_direction = function get_direction() {
	return this.gpio.get_direction();
};

Gpio.prototype.get_id = function get_id() {
	return this.gpio.get_id();
};
