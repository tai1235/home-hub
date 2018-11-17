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

/* Test Specific Includes */
var bt_started;
var remote_addr = process.env.BT_ADDR;
var uuid = process.env.BT_PROFILE_UUID
var runManualTests = parseInt(process.env.RUN_MANUAL_TESTS);

/* Test Case Module */
testCase('Bluetooth', function() {

	pre(function() {
	});

	testCase('#on(started)', function() {
		assertions('Return callback event when the bluetooth interface is started', function(done) {
			if (!runManualTests)
				this.skip();

			bt = new (require('../src/bluetooth'))();
			this.timeout(10000);
			bt.on('started', function() {
				console.log('onstarted');
				bt.removeAllListeners();
				done();
			});
		});

	});

	testCase('#start_scan(), on(scan)', function() {
		postEach(function() {
			if (!runManualTests)
				this.skip();

			bt.stop_scan();
			bt.removeAllListeners()
		});

		assertions('Return callback event when the bluetooth scan request is performed', function(done) {
			if (!runManualTests)
				this.skip();

			this.timeout(20000);

			if (!remote_addr || !remote_addr.length)
				this.skip();

			bt.on('scan', function(err, device) {
				console.log('scanned');
				assert.isNull(err);
				assert.isNotNull(device);
				device = JSON.parse(device);
				console.log("device addr = " + device[0].address);
				console.log("device remote addr = " + remote_addr);
				if (device[0].address.toUpperCase() == remote_addr.toUpperCase()) {
					console.log("Remote device found");
					done();
				}
			});

			bt.start_scan();
		});

		assertions('Apply filter with rssi equal to -70', function(done) {
			if (!runManualTests)
				this.skip();

			this.timeout(10000);
			if (!remote_addr || !remote_addr.length)
				this.skip();

			var filter = new (require('../src/bluetooth').Filter)(-70, [], "auto");
			bt.set_scan_filter(filter);
			bt.on('scan', function(err, device) {
				assert.isOk(bt.is_scanning());
				assert.isNull(err);
				assert.isNotNull(device);
				device = JSON.parse(device);
				assert.isAtLeast(device[0].rssi, filter.rssi);
				if (device[0].address.toUpperCase() == remote_addr.toUpperCase()) {
					console.log("Remote device found");
					done();
				}
			});
			assert.isNotOk(bt.is_scanning());
			bt.start_scan();
		});

		assertions('Apply filter with nonempty uuids', function(done) {
			if (!remote_addr || !remote_addr.length || !runManualTests)
				this.skip();

			if (!uuid || !uuid.length)
				this.skip();

			this.timeout(10000);
			var filter =
				new (require('../src/bluetooth').Filter)(
					-100,
					[uuid],
					"auto");
			bt.set_scan_filter(filter);
			bt.on('scan', function(err, device) {
				assert.isOk(bt.is_scanning());
				assert.isNull(err);
				assert.isNotNull(device);
				device = JSON.parse(device);

				uuids = [];
				for (var i = 0, len = device[0].uuids.length; i < len; i++) {
					uuids.push(device[0].uuids[i].uuid);
				}
				assert.include(uuids, uuid);
				if (device[0].address.toUpperCase() == remote_addr.toUpperCase()) {
					console.log("Remote device found");
					done();
				}
			});
			assert.isNotOk(bt.is_scanning());
			bt.start_scan();
		});
	});

	testCase('#start_bond(), on(bond)', function() {

		assertions('Return callback event when the bluetooth interface is bonded to remote device', function(done) {
			if (!remote_addr || !remote_addr.length || !runManualTests)
				this.skip();

			this.timeout(10000);
			bt.on('bond', function(err, paired) {
				console.log('bonded');
				assert.isNull(err);
				assert.isDefined(paired);
				console.log('device is ' + paired + ' with ' + remote_addr)
				assert.isOk(bt.is_paired(remote_addr));
				done();
			});
			bt.start_bond(remote_addr);
		});

	});

	testCase('#connect(), on(connect)', function() {

		assertions('Return callback event when the bluetooth interface is connected to remote device', function(done) {
			if (!remote_addr || !remote_addr.length || !runManualTests)
				this.skip();

			this.timeout(10000);
			bt.on('connect', function(err, connected) {
				assert.isNull(err);
				assert.isDefined(connected);
				console.log('connected');
				assert.isOk(bt.is_connected(remote_addr));
				bt.removeAllListeners();
				done();
			});
			bt.connect(remote_addr);
		});

	});

	testCase('#get_adapter_info', function() {
		assertions('Get information about the bluetooth adapter', function() {
			if (!runManualTests)
				this.skip();

			var cmd = "hciconfig | grep Address | awk '{print $3}'";
			var exec = require('child_process').execSync;
			var addr = exec(cmd);
			var adapter = bt.get_adapter_info();
			addr = addr.toString().replace(/\n$/, '');

			assert.isString(adapter.address);
			assert.equal(addr, adapter.address);
			assert.isString(adapter.name);
			assert.isString(adapter.alias);
			assert.equal(adapter.name, adapter.alias);

			assert.isObject(adapter.cod);
			assert.isString(adapter.cod.major);
			assert.isString(adapter.cod.minor);
			assert.isString(adapter.cod.service_class);

			assert.isBoolean(adapter.discoverable);
			assert.isBoolean(adapter.pairable);
			assert.isNumber(adapter.pair_timeout);
			assert.isNumber(adapter.discover_timeout);
			assert.isBoolean(adapter.discovering);
			assert.isArray(adapter.uuids);
			assert.isObject(adapter.uuids[0]);
			assert.isString(adapter.uuids[0].uuid);
			assert.isString(adapter.uuids[0].uuid_name);
		});
	});

	testCase('#set_alias', function() {
		assertions('Set the property alias', function(done) {
			if (!runManualTests)
				this.skip();

			bt.set_alias("ARTIK-BLUETOOTH-TEST");
			setTimeout(function() {
				var adapter = bt.get_adapter_info();
				assert.equal("ARTIK-BLUETOOTH-TEST", adapter.alias);
				bt.set_alias("");
			}, 100);
			setTimeout(function() {
						var adapter = bt.get_adapter_info();
				assert.equal(adapter.name, adapter.alias);
				done();
			}, 200);
		});
	});

	testCase('#set_discoverable', function() {
		assertions('Set the property discoverable', function(done) {
			if (!runManualTests)
				this.skip();

			bt.set_discoverable(true);
			setTimeout(function() {
				var adapter = bt.get_adapter_info();
				assert.isOk(adapter.discoverable);
				bt.set_discoverable(false);
			}, 100);
			setTimeout(function() {
				var adapter = bt.get_adapter_info();
				assert.isNotOk(adapter.discoverable);
				done();
			}, 300);
		});

		assertions('Set the property discoverable to true during 1s', function(done) {
			if (!runManualTests)
				this.skip();

			bt.set_discoverableTimeout(1);
			bt.set_discoverable(true);

			setTimeout(function() {
				var adapter = bt.get_adapter_info();
				assert.isOk(adapter.discoverable);
				assert.equal(1, adapter.discover_timeout);
			}, 100);
			setTimeout(function() {
				var adapter = bt.get_adapter_info();
				assert.isNotOk(adapter.discoverable);
				assert.equal(1, adapter.discover_timeout);
				done();
			}, 1100);
		});
	});

	testCase('#set_pairable', function(){
		assertions('Set the property pairable', function(done) {
			if (!runManualTests)
				this.skip();

			bt.set_pairableTimeout(0);
			bt.set_pairable(true);
			setTimeout(function() {
				var adapter = bt.get_adapter_info();
				assert.isOk(adapter.pairable);
				bt.set_pairable(false);
			}, 100);
			setTimeout(function() {
				var adapter = bt.get_adapter_info();
				assert.isNotOk(adapter.pairable);
				done();
			}, 300);
		});

		assertions('Set the property pairable to true during 1s', function(done) {
			if (!runManualTests)
				this.skip();

			bt.set_pairableTimeout(1);
			bt.set_pairable(true);

			setTimeout(function() {
				var adapter = bt.get_adapter_info();
				assert.isOk(adapter.pairable);
				assert.equal(1, adapter.pair_timeout);
			}, 100);
			setTimeout(function() {
				var adapter = bt.get_adapter_info();
				assert.isNotOk(adapter.pairable);
				assert.equal(1, adapter.pair_timeout);
				done();
			}, 1100);
		});
	});

	testCase('#connect_profile', function(){
		assertions('Connect to a specific profile', function(done) {
			if (!remote_addr || !remote_addr.length || !runManualTests)
				this.skip();

			if (!uuid || !uuid.length)
				this.skip();

			this.timeout(10000);
			bt.on('connect', function(err, connected) {
				assert.isNull(err);
				assert.isDefined(connected);
				console.log('connected');
				assert.isOk(bt.is_connected(remote_addr));
				bt.removeAllListeners();
				done();
			});

			bt.connect_profile(remote_addr, uuid);
		});
	});

	testCase('#set_trust', function(){
		assertions('Set the device property trust', function(done) {
			if (!remote_addr || !remote_addr.length || !runManualTests)
				this.skip();

			/* This test does not wrok */
			/* It is unable to set trust to true */
			this.skip();
			bt.set_trust(remote_addr);
			setTimeout(function() {
				console.log("set trust to false");
				assert.isOk(bt.is_trusted(remote_addr));
				bt.unset_trust(remote_addr);
			}, 1000);
			setTimeout(function() {
				assert.isNotOk(bt.is_trusted(remote_addr));
				done();
			}, 1500);
		});
	});

	testCase('#set_block', function(){
		assertions('Set the device property block', function(done) {
			if (!remote_addr || !remote_addr || !runManualTests)
				this.skip();

			bt.set_block(remote_addr);
			setTimeout(function() {
				assert.isOk(bt.is_blocked(remote_addr));
				bt.unset_block(remote_addr);
			}, 100);
			setTimeout(function() {
				assert.isNotOk(bt.is_blocked(remote_addr));
				done();
			}, 00);
		});
	});

	post(function() {
		if (remote_addr && remote_addr.length && runManualTests)
			bt.remove_device(remote_addr);
	});

});
