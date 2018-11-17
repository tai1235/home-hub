var events = require('events');
var util = require('util');
var artik = require('../../build/Release/artik-sdk.node');

var Service = function(options) {
	this.uuid = options.uuid;
	this.characteristics = options.characteristics || [];
	this.service = new artik.Service(this.uuid, this.characteristics.map(function(x){ return x.characteristic}));
}

function Characteristic(options) {
	this.uuid = options.uuid;
	this.properties = options.properties || [];
	this.value = options.value || null;
	this.descriptors = options.descriptors || [];

	if (options.onReadRequest) {
		this.onReadRequest = options.onReadRequest;
	}

	if (options.onWriteRequest) {
		this.onWriteRequest = options.onWriteRequest;
	}

	if (options.onSubscribe) {
		this.onSubscribe = options.onSubscribe;
	}

	if (options.onUnsubscribe) {
		this.onUnsubscribe = options.onUnsubscribe;
	}

	this.characteristic =
		new artik.Characteristic(this.uuid,
					 this.properties,
					 this.value,
					 this.descriptors.map(function(x){ return x.descriptor; }));

	this.characteristic.on('readRequest', this.onReadRequestInternal.bind(this));
	this.characteristic.on('writeRequest', this.onWriteRequestInternal.bind(this));
	this.characteristic.on('notifyRequest', this.onNotifyInternal.bind(this));

	this.on("readRequest", this.onReadRequest.bind(this));
	this.on("writeRequest", this.onWriteRequest.bind(this));
	this.on("subscribe", this.onSubscribe.bind(this));
	this.on("unsubscribe", this.onUnsubscribe.bind(this));

	this.gatt_server = new artik.GattServer();
}

util.inherits(Characteristic, events.EventEmitter);

Characteristic.prototype.onReadRequest = function(callback) {
	if (this.value)
		callback("ok", this.value);
	else
		callback("notSupported", null);
};


Characteristic.prototype.onWriteRequest = function(data, callback) {
	callback("notSupported");
};

Characteristic.prototype.onSubscribe = function(updateValueCallback) {
	this.updateValueCallback = updateValueCallback;
};

Characteristic.prototype.onUnsubscribe = function() {
	this.updateValueCallback = null;
};

Characteristic.prototype.onReadRequestInternal = function(request) {
	var _ = this;
	var callback = function(result, value) {
		if (result == "ok")
			_.gatt_server.request_send_value(request, value);
		else
			_.gatt_server.request_send_result(request, result, null);
	}
	this.emit("readRequest", callback);
}

Characteristic.prototype.onWriteRequestInternal = function(request, buffer) {
	var _ = this;
	var callback = function(result) {
		if (result == "ok")
			_.value = buffer;
		_.gatt_server.request_send_result(request, result, null);
	}

	this.emit("writeRequest", buffer, callback);
}

Characteristic.prototype.onNotifyInternal = function(service_id, characteristic_id, state) {
	var _ = this;
	var callback = function(value) {
		console.log("update value");
		_.value = value;
		_.gatt_server.notify(service_id, characteristic_id, value);
	}

	if (state)
		this.emit("subscribe", callback);
	else
		this.emit("unsubscribe");
}

var Descriptor = function(options) {
	this.uuid = options.uuid;
	this.properties = options.properties || [];
	this.value = options.value || null;

	if (options.onReadRequest) {
		this.onReadRequest = options.onReadRequest;
	}

	if (options.onWriteRequest) {
		this.onWriteRequest = options.onWriteRequest;
	}

	this.descriptor = new artik.Descriptor(this.uuid, this.properties, this.value);
	this.descriptor.on('readRequest', this.onReadRequestInternal.bind(this));
	this.descriptor.on('writeRequest', this.onWriteRequestInternal.bind(this));

	this.on("readRequest", this.onReadRequest.bind(this));
	this.on("writeRequest", this.onWriteRequest.bind(this));

	this.gatt_server = new artik.GattServer();
}

util.inherits(Descriptor, events.EventEmitter);

Descriptor.prototype.onReadRequestInternal = function(request) {
	var _ = this;
	var callback = function(result, value) {
		if (result == "ok")
			_.gatt_server.request_send_value(request, value);
		else
			_.gatt_server.request_send_result(request, result, null);
	}

	this.emit("readRequest", callback);
}

Descriptor.prototype.onWriteRequestInternal = function(request, buffer) {
	var _ = this;
	var callback = function(result) {
		if (result == "ok")
		{
			_.value = buffer;
		}
		_.gatt_server.request_send_result(request, result, null);
	}

	this.emit("writeRequest", buffer, callback);
}

Descriptor.prototype.onReadRequest = function(callback) {
	if (this.value)
		callback("ok", this.value);
	else
		callback("notSupported", null);
};

Descriptor.prototype.onWriteRequest = function(data, callback) {
	callback("notSupported");
};

var GattServer = function()
{
	this.gattserver = new artik.GattServer();
	this.services = [];
}

GattServer.prototype.start_advertising = function(options)
{
	return this.gattserver.start_advertising(options);
}

GattServer.prototype.stop_advertising = function(advertising_id)
{
	return this.gattserver.stop_advertising(advertising_id);
}

GattServer.prototype.add_service = function(options)
{
	this.services.push(new Service(options));
}

artik.Characteristic.prototype.__proto__ = events.EventEmitter.prototype;
artik.Descriptor.prototype.__proto__ = events.EventEmitter.prototype;

module.exports = GattServer;
module.exports.Characteristic = Characteristic;
module.exports.Descriptor = Descriptor;
