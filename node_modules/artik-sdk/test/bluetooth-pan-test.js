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
var bluetooth  = require('../src/bluetooth');
var network    = require('../src/network');
var runManualTests = parseInt(process.env.RUN_MANUAL_TESTS);

var remote_addr = process.env.BT_ADDR;
var panu = process.env.BT_PAN_IS_PANU;
var is_panu = !remote_addr || !remote_addr.length || panu;

var pan = new bluetooth.Pan();
var bt = new bluetooth();

testCase('Pan', function() {
	testCase('#PAN as NAP', function() {
		assertions('Register and unregister', function() {
			if (is_panu || !runManualTests)
				this.skip();

			pan.register("nap", "artik-bridge");
			pan.unregister("nap");
		});
	});

	testCase('PAN as PANU', function() {
		pre(function() {
			if (!runManualTests)
				this.skip();

			exec("systemctl stop connman");
		});

		assertions('Test connectivity', function(done) {
			if (!is_panu || !runManualTests)
				this.skip();

			this.timeout(100000);
			bt.on('bond', function(err, paired) {
				if (paired) {
					var net = new network();
					net_interface = pan.connect(remote_addr, "nap");
					console.log("net_interface = " + net_interface);
					exec("dhclient -r " + net_interface);
					exec("dhclient " + net_interface);
					exec("ifconfig eth0 down");

					assert.isOk(pan.get_connected());
					assert.isString(pan.get_interface());
					assert.isString(pan.get_UUID());

					assert.isOk(net.get_online_status());

					exec("ifconfig eth0 up");
					pan.disconnect();
					done();
				}
			});
			bt.start_bond(remote_addr);
		});

		post(function() {
			if (!runManualTests)
				this.skip();

			exec("systemctl start connman");
		});
	});
});
