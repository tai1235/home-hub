var events = require('events');
var util = require('util');
var websocket = require('../build/Release/artik-sdk.node').websocket;

var Websocket = function(uri, ssl_config) {
	events.EventEmitter.call(this);
	this.websocket = websocket(uri, ssl_config);
}

util.inherits(Websocket, events.EventEmitter);

module.exports = Websocket;

Websocket.prototype.open_stream = function open_stream() {
	var _ = this;
	return this.websocket.open_stream(
		function(status) {
			_.emit('connected', status);
		}, 
		function(message) {
			_.emit('receive', message);
		});
};

Websocket.prototype.write_stream = function write_stream(message) {
	return this.websocket.write_stream(message);
};

Websocket.prototype.close_stream = function close_stream() {
	return this.websocket.close_stream();
};
