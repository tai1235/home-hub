var events = require('events');
var util = require('util');
var artik = require('../../build/Release/artik-sdk.node');
var bluetooth = artik.bluetooth;

var Bluetooth = function(){
	events.EventEmitter.call(this);
	this.bt = new bluetooth();
	setImmediate(function(self) {
		self.emit('started');
	}, this);
}

util.inherits(Bluetooth, events.EventEmitter);

Bluetooth.prototype.start_scan = function() {
	var _ = this;
	return this.bt.start_scan(function(err, device) {
		_.emit('scan', err, device);
	});
};

Bluetooth.prototype.stop_scan = function() {
	return this.bt.stop_scan();
};

Bluetooth.prototype.get_devices = function(type) {
	return this.bt.get_devices(type);
};

Bluetooth.prototype.start_bond = function(addr) {
	var _ = this;
	return this.bt.start_bond(addr, function(err, status) {
		_.emit('bond', err, status);
	});
};

Bluetooth.prototype.stop_bond = function(addr) {
	return this.bt.stop_bond(addr);
};

Bluetooth.prototype.connect = function(addr) {
	var _ = this;
	return this.bt.connect(addr, function(err, status) {
		_.emit('connect', err, status);
	});
};

Bluetooth.prototype.disconnect = function(addr) {
	return this.bt.disconnect(addr);
};

Bluetooth.prototype.remove_unpaired_devices = function() {
	return this.bt.remove_unpaired_devices();
};

Bluetooth.prototype.remove_device = function(addr) {
	return this.bt.remove_device(addr);
};

Bluetooth.prototype.set_scan_filter = function(filter) {
	return this.bt.set_scan_filter(filter)
}

Bluetooth.prototype.get_adapter_info = function() {
	return this.bt.get_adapter_info();
}

Bluetooth.prototype.set_alias = function(alias) {
	return this.bt.set_alias(alias);
}

Bluetooth.prototype.set_discoverable = function(val) {
	return this.bt.set_discoverable(val);
}

Bluetooth.prototype.set_discoverableTimeout = function(val) {
	return this.bt.set_discoverableTimeout(val);
}

Bluetooth.prototype.set_pairable = function(val) {
	return this.bt.set_pairable(val);
}

Bluetooth.prototype.set_pairableTimeout = function(val) {
	return this.bt.set_pairableTimeout(val);
}

Bluetooth.prototype.connect_profile = function(addr, uuid) {
	return this.bt.connect_profile(addr, uuid);
}

Bluetooth.prototype.is_scanning = function() {
	return this.bt.is_scanning();
}

Bluetooth.prototype.is_paired = function(addr) {
	return this.bt.is_paired(addr);
}

Bluetooth.prototype.is_connected = function(addr) {
	return this.bt.is_connected(addr);
}

Bluetooth.prototype.is_trusted = function(addr) {
	return this.bt.is_trusted(addr);
}

Bluetooth.prototype.is_blocked = function(addr) {
	return this.bt.is_blocked(addr);
}

Bluetooth.prototype.set_trust = function(addr) {
	return this.bt.set_trust(addr);
}

Bluetooth.prototype.unset_trust = function(addr) {
	return this.bt.unset_trust(addr);
}

Bluetooth.prototype.set_block = function(addr) {
	return this.bt.set_block(addr);
}

Bluetooth.prototype.unset_block = function(addr) {
	return this.bt.unset_block(addr);
}

module.exports = Bluetooth;
