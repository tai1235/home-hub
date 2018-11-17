/* Global Includes */
var testCase   = require('mocha').describe;
var pre        = require('mocha').before;
var preEach    = require('mocha').beforeEach;
var post       = require('mocha').after;
var postEach   = require('mocha').afterEach;
var assertions = require('mocha').it;
var assert     = require('chai').assert;
var validator  = require('validator');
var exec       = require('child_process').execSync;
var artik      = require('../src/wifi');

var wifi_station;

/* Test Specific Includes */
var ssid = process.env.WIFI_SSID;
var pwd  = process.env.WIFI_PSK;

/* Test Case Module */
testCase('Wifi', function() {

	pre(function() {
	});


	testCase('#on(started)', function() {

		assertions('Return callback event when the wifi interface is started', function(done) {
			wifi_station = new artik.wifi_station();
			this.timeout(10000);
			wifi_station.on('started', function() {
				console.log('onstarted');
				done();
			});

		});

	});

	testCase('#get_info()', function() {

		assertions('Returns current wifi station information', function(done) {

			var info = JSON.parse(wifi_station.get_info());
			assert.isNotNull(info);
			console.log(info);
			done();

		});

	});

	testCase('#scan_request(), on(scan)', function() {

		assertions('Return callback event when the wifi scan request is performed', function(done) {
			this.timeout(10000);
			wifi_station.on('scan', function(list) {
				console.log('onscan');
				assert.isNotNull(list);
				done();
			});

			wifi_station.scan_request();
		});

	});

	testCase('#connect(), on(connected)', function() {


		assertions('Return callback event when the wifi interface is connected to AP', function(done) {

			if (!ssid || !pwd || !ssid.length || !pwd.length)
				this.skip();

			this.timeout(10000);
			wifi_station.on('connected', function(result) {
				console.log('onconnected');
				done();
			});

			wifi_station.disconnect();
			wifi_station.connect(ssid, pwd, true);

		});

	});

	post(function() {
	});

});
