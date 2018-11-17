var events = require('events');
var util = require('util');
var artik = require('../../build/Release/artik-sdk.node');
var agent = artik.Agent;

var Agent = function() {
	var _ = this;
	var release_event = function() {
		_.emit('release');
	};
	var request_pincode_event = function(request, device) {
		_.emit('request_pincode', request, device);
	};
	var display_pincode_event = function(device, pincode) {
		_.emit('display_pincode', device, pincode);
	};
	var request_passkey_event = function(request, device) {
		_.emit('request_passkey', request, device);
	};
	var display_passkey_event = function(device, passkey, entered) {
		_.emit('display_passkey', device, passkey, entered);
	};
	var request_confirmation_event = function(request, device, passkey) {
		_.emit('confirmation', request, device, passkey);
	};
	var request_authorization_event = function(request, device) {
		_.emit('authorization', request, device);
	};
	var request_authorize_service_event = function(request, device, uuid) {
		_.emit('authorize_service', request, device, uuid);
	};
	var cancel_event = function() {
		_.emit('cancel');
	};

	this.agent = new agent(
		release_event,
		request_pincode_event,
		display_pincode_event,
		request_passkey_event,
		display_passkey_event,
		request_confirmation_event,
		request_authorization_event,
		request_authorize_service_event,
		cancel_event
	);

	events.EventEmitter.call(this);
}

util.inherits(Agent, events.EventEmitter);

Agent.prototype.register_capability = function(capability) {
	return this.agent.register_capability(capability);
}

Agent.prototype.set_default = function() {
	return this.agent.set_default();
}

Agent.prototype.unregister = function() {
	return this.agent.unregister();
}

module.exports = Agent;
