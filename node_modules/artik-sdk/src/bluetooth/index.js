var events = require('events');
var util = require('util');
var artik = require('../../build/Release/artik-sdk.node');

var Filter = function(rssi, uuids, type) {
	this.rssi = rssi;
	this.uuids = uuids;
	this.type = type;
}

var SppProfile = function(
	name,
	service,
	role,
	channel,
	PSM,
	require_authentication,
	require_authorization,
	auto_connect,
	version,
	features)
{
	this.name = name;
	this.service = service;
	this.role = role;
	this.channel = channel;
	this.PSM = PSM;
	this.require_authentication = require_authentication;
	this.require_authorization = require_authorization;
	this.auto_connect = auto_connect;
	this.version = version;
	this.features = features;
}

// inherits() does not work in node@4 with binding types
artik.SppSocket.prototype.__proto__ = events.EventEmitter.prototype;
artik.Spp.prototype.__proto__ = events.EventEmitter.prototype;
artik.Ftp.prototype.__proto__ = events.EventEmitter.prototype;

module.exports = require('./bluetooth');
module.exports.Avrcp = artik.Avrcp;
module.exports.A2dp = artik.A2dp;
module.exports.Pan = artik.Pan;
module.exports.Spp = artik.Spp;
module.exports.Ftp = artik.Ftp;
module.exports.Filter = Filter;
module.exports.SppProfile = SppProfile;
module.exports.Agent = require('./agent');
module.exports.GattServer = require('./gattserver');
module.exports.GattClient = require('./gattclient');
