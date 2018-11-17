var events = require('events');
var util = require('util');
var mqtt = require('../build/Release/artik-sdk.node').mqtt;

var Mqtt = function(client_id, user_name, user_password, clean, keep_alive, block, ssl_config) {
    events.EventEmitter.call(this);
    _ = this;
    this.mqtt = new mqtt(client_id, user_name, user_password, clean, keep_alive, block, ssl_config,
			 function(result) {
    			     _.emit('connected', result);
			 },
			 function(result) {
    			     _.emit('disconnected', result);
			 },
			 function(mid) {
    			     _.emit('subscribed', mid);
			 },
			 function(mid) {
    			     _.emit('unsubscribed', mid);
			 },
			 function(mid) {
    			     _.emit('published', mid);
			 },
			 function(mid, topic, buffer, qos, retain) {
    			     _.emit('received', mid, topic, buffer, qos, retain);
			 });
    setImmediate(function() {
	_.emit('started');
    });
}

util.inherits(Mqtt, events.EventEmitter);

module.exports = Mqtt;

Mqtt.prototype.set_willmsg = function(topic, message, qos, retain) {
    return this.mqtt.set_willmsg(topic, message, qos, retain);
}

Mqtt.prototype.free_willmsg = function() {
    return this.mqtt.free_willmsg();
}

Mqtt.prototype.clear_willmsg = function() {
    return this.mqtt.clear_willmsg();
}

Mqtt.prototype.connect = function(host, port, callback) {
    return this.mqtt.connect(host, port, callback);
}

Mqtt.prototype.disconnect = function(callback) {
    return this.mqtt.disconnect(callback);
}

Mqtt.prototype.subscribe = function(qos, topic, callbackSubscribe, callbackReceive) {
    return this.mqtt.subscribe(qos, topic, callbackSubscribe, callbackReceive);
}

Mqtt.prototype.unsubscribe = function(topic, callback) {
    return this.mqtt.unsubscribe(topic, callback);
}

Mqtt.prototype.publish = function(qos, retain, topic, buffer, callbackPublish) {
    return this.mqtt.publish(qos, retain, topic, buffer, callbackPublish);
}
