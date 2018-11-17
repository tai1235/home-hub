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
var fs         = require('fs');

/* Test Specific Includes */
var cloud             = require('../src/cloud');
var auth_token        = process.env.WEBSOCKET_ENABLE_SDR == 1 ? process.env.WEBSOCKET_SDR_ACCESS_TOKEN : process.env.WEBSOCKET_ACCESS_TOKEN;
var device_id         = process.env.WEBSOCKET_ENABLE_SDR == 1 ? process.env.WEBSOCKET_SDR_DEVICE_ID : process.env.WEBSOCKET_DEVICE_ID;
var use_se            = process.env.WEBSOCKET_ENABLE_SDR == 1 ? true : false;
var test_message      = process.env.WEBSOCKET_ENABLE_SDR == 1 ? process.env.WEBSOCKET_SDR_MESSAGE : process.env.WEBSOCKET_MESSAGE;

var conn = new cloud();

var data = "";

if (process.env.CLOUD_CA_ROOT_FILE != "")
	data = fs.readFileSync(process.env.CLOUD_CA_ROOT_FILE);

var ssl_config = {
	ca_cert: Buffer.from(data),
	verify_cert: process.env.CLOUD_VERIFY_CERT == 1 ? "required" : "none",
	se_config:  (process.env.WEBSOCKET_ENABLE_SDR == 1 ? { certificate_identifier: 'artik' }: undefined )
}

/* Test Case Module */
testCase('Cloud-Websockets', function() {

	this.timeout(10000);

	pre(function() {

	});

	testCase('#websocket_open_stream(), on(receive)', function() {

		assertions('Return callback event when the websocket is connected', function(done) {

			function on_received (message) {
				console.log("received: " + message);
				assert.isNotNull(message);
				console.log(JSON.parse(message));
				if (JSON.parse(message).type != "ping") {
					assert.equal(JSON.parse(message).data.code, "200");
					assert.equal(JSON.parse(message).data.message, "OK");
					conn.removeListener('receive', on_received);
					done();
				}
			}

			conn.on('receive', on_received);

			conn.websocket_open_stream(auth_token, device_id, ssl_config);
		});

	});

	testCase('#websocket_write_stream(), on(receive)', function() {

		assertions('Return callback event when the data is received', function(done) {

            if (!auth_token || !device_id || !auth_token.length || !device_id.length)
                this.skip();

			conn.on('receive', function(message) {
				console.log("received: " + message);
				assert.isNotNull(message);
				assert.isNotNull(JSON.parse(message).data.mid);
				done();
			});

			console.log("sending: " + test_message);
			conn.websocket_send_message(test_message);
		});

	});

	post(function() {
	});

});
