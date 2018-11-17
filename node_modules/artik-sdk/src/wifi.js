var events = require('events');
var util = require('util');
var wifi = require('../build/Release/artik-sdk.node').wifi;

var Wifi_AP = function(){
	events.EventEmitter.call(this);
	this.wifi = new wifi("ap");
	setImmediate(function(self) {
		self.emit('started');
	}, this);
}

var Wifi_Station = function(){
	events.EventEmitter.call(this);
	this.wifi = new wifi("station");
	setImmediate(function(self) {
		self.emit('started');
	}, this);
}

util.inherits(Wifi_AP, events.EventEmitter);
util.inherits(Wifi_Station, events.EventEmitter);

module.exports = {
	wifi_ap : Wifi_AP,
	wifi_station : Wifi_Station
}

Wifi_AP.prototype.WIFI_ENCRYPTION_OPEN = 0x00000000;
Wifi_AP.prototype.WIFI_ENCRYPTION_WEP = 0x00000001;
Wifi_AP.prototype.WIFI_ENCRYPTION_WPA = 0x00000002;
Wifi_AP.prototype.WIFI_ENCRYPTION_WPA2 = 0x00000004;
Wifi_AP.prototype.WIFI_ENCRYPTION_WPA2_PERSONAL = 0x00010000;
Wifi_AP.prototype.WIFI_ENCRYPTION_WPA2_ENTERPRISE = 0x00020000;

Wifi_Station.prototype.scan_request = function() {
	var _ = this;
	return this.wifi.scan_request(function(list) {
		_.emit('scan', list);
	});
};

Wifi_Station.prototype.connect = function(ssid, password, is_persistent) {
	var _ = this;
	return this.wifi.connect(ssid, password, is_persistent, function(result) {
		if (result)
			_.emit('connected');
	});
};

Wifi_Station.prototype.disconnect = function() {
	return this.wifi.disconnect();
};

Wifi_Station.prototype.get_scan_result = function() {
	return this.wifi.get_scan_result();
};

Wifi_Station.prototype.get_info = function() {
	return this.wifi.get_info();
};

Wifi_AP.prototype.start_ap = function(ssid, password, channel, encryption) {
	return this.wifi.start_ap(ssid, password, channel, encryption);
};
