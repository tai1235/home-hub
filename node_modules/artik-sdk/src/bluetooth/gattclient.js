var events = require('events');
var util = require('util');
var artik = require('../../build/Release/artik-sdk.node');

artik.RemoteCharacteristic.prototype.__proto__ = events.EventEmitter.prototype;
artik.GattClient.prototype.__proto__ = events.EventEmitter.prototype;

module.exports = artik.GattClient;
