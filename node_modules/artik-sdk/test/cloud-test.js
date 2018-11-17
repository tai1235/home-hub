/* Global Includes */
var fs 				 = require('fs');
var testCase   = require('mocha').describe;
var pre        = require('mocha').before;
var preEach    = require('mocha').beforeEach;
var post       = require('mocha').after;
var postEach   = require('mocha').afterEach;
var assertions = require('mocha').it;
var assert     = require('chai').assert;
var validator  = require('validator');
var exec       = require('child_process').execSync;
var artik      = require('../src');
var artik_http = require("../src/http");
var http 			 = new artik_http();

/* Test Specific Includes */
var action = process.env.CLOUD_ACTION;
var user_id = process.env.CLOUD_USER_ID;
var message = process.env.CLOUD_MESSAGE;
var device_id = process.env.CLOUD_DEVICE_ID;
var server_props = process.env.CLOUD_SERVER_PROPS;
var device_token = process.env.CLOUD_DEVICE_TOKEN;
var device_type_id = process.env.CLOUD_DEVICE_TYPE_ID;
var timestamp = process.env.CLOUD_ENABLE_TIMESTAMP == 1 ? true : false;

var cloud2 = new artik.cloud(device_token);

var device_id_1;
var device_id_2;

var data = "";

if (process.env.CLOUD_CA_ROOT_FILE != "")
	data = fs.readFileSync(process.env.CLOUD_CA_ROOT_FILE);

var ssl_config = {
	ca_cert: Buffer.from(data),
	verify_cert: process.env.CLOUD_VERIFY_CERT == 1 ? "required" : "none",
	se_config:  (process.env.WEBSOCKET_ENABLE_SDR == 1 ? { certificate_identifier: 'artik' }: undefined )
}

var response = cloud2.get_device_properties(device_id, timestamp, ssl_config);
var akc_auth_token = JSON.parse(response).data.serverProperties.akc_auth_token;
var akc_basic_token = JSON.parse(response).data.serverProperties.akc_basic_token;
var akc_refresh_token = JSON.parse(response).data.serverProperties.akc_refresh_token;
var akc_expires_at = JSON.parse(response).data.serverProperties.akc_expires_at;

var cloud	= new artik.cloud(akc_auth_token);

var date = new Date().getTime();
time = Math.floor(date/1000);

if (akc_expires_at < time) {

	console.log("akc_auth_token is expired");

	/* Refresh AKC API TOKEN */
	var headers = [ "Authorization", "Basic " + akc_basic_token ];
	var url = "https://accounts.artik.cloud/token?grant_type=refresh_token&refresh_token=" + akc_refresh_token;

	http.post(url, headers, null, ssl_config, function(response, status) {

		if (status != 200) {
			console.log("Post failed");
			process.exit(-1);
		}

		/* SET SERVER PROPERTIES WITH NEW akc_auth_token, akc_refresh_token, akc_expires_at */
		akc_auth_token = JSON.parse(response).access_token;
		akc_refresh_token = JSON.parse(response).refresh_token;
		akc_expires_at = time + JSON.parse(response).expires_in;

		if (!device_id || !server_props || !device_id.length || !server_props.length) {
			console.log("Error to update server properties");
			process.exit(-1);
		}

		var server_properties = {
			akc_auth_token: akc_auth_token,
			akc_refresh_token: akc_refresh_token,
			akc_expires_at: akc_expires_at,
		};

		cloud2.set_device_server_properties(device_id, JSON.stringify(server_properties), function(err, response) {
			console.log(err);
			console.log(response);
			cloud	= new artik.cloud(akc_auth_token);
		});
	});
}


/* Test Case Module */
testCase('Cloud', function() {

	this.timeout(6000);

	pre(function() {
	});

	testCase('#get_current_user_profile()', function() {

		assertions('Get User Profile - Should return ARTIK Cloud User profile', function(done) {
			if (!akc_auth_token || !akc_auth_token.length)
				this.skip();

			var response = cloud.get_current_user_profile(ssl_config);
			assert.notInclude(response, "error");
			user_id = JSON.parse(response).data.id;
			done();
		});

		assertions('Get User Profile Callback - Returns user profile response in the callback', function(done) {

			if (!akc_auth_token || !akc_auth_token.length)
				this.skip();

			cloud.get_current_user_profile(function(err, response) {
				assert.isNull(err);
				assert.isObject(response);
				console.log(JSON.stringify(response));
				done();
			}, ssl_config);
		});
	});

	testCase('#get_device_token()', function() {

		assertions('Get Device Token - Should return the ARTIK cloud device token ', function(done) {

			if (!akc_auth_token || !device_id || !akc_auth_token.length || !device_id.length)
				this.skip();

			assert.notInclude(cloud.get_device_token(device_id, ssl_config), "error");
			done();
		});

		assertions('Get Device Token Callback - Should return the callback with ARTIK cloud device token', function(done) {

			if (!akc_auth_token || !device_id || !akc_auth_token.length || !device_id.length)
				this.skip();

			cloud.get_device_token(device_id, function(err, response) {
				assert.isNull(err);
				assert.isObject(response);
				console.log(JSON.stringify(response));
				done();
			}, ssl_config);
		});

	});

	testCase('#get_device()', function() {

		assertions('Get Device without properties ', function(done) {

			if (!akc_auth_token || !device_id || !akc_auth_token.length || !device_id.length)
				this.skip();
			assert.notInclude(cloud.get_device(device_id, false, ssl_config), "error");
			done();
		});

		assertions('Get Device with properties ', function(done) {

			if (!akc_auth_token || !device_id || !akc_auth_token.length || !device_id.length)
				this.skip();

			assert.notInclude(cloud.get_device(device_id, true, ssl_config), "error");
			done();
		});

		assertions('Get Device without properties - Callback', function(done) {

			if (!akc_auth_token || !device_id || !akc_auth_token.length || !device_id.length)
				this.skip();

			cloud.get_device(device_id, false, function(err, response) {
				assert.isNull(err);
				assert.isObject(response);
				console.log(JSON.stringify(response));
				done();
			}, ssl_config);
		});

		assertions('Get Device with properties - Callback', function(done) {

			if (!akc_auth_token || !device_id || !akc_auth_token.length || !device_id.length)
				this.skip();

			cloud.get_device(device_id, true, function(err, response) {
				assert.isNull(err);
				assert.isObject(response);
				console.log(JSON.stringify(response));
				done();
			}, ssl_config);
		});

	});

	testCase('#get_user_device_types()', function() {

		assertions('Get User Device Types ', function(done) {

			if (!akc_auth_token || !device_id || !akc_auth_token.length || !device_id.length)
				this.skip();

			assert.notInclude(cloud.get_user_device_types(100, false, 0, user_id, ssl_config), "error");
			done();
		});

		assertions('Get User Device Types - Callback', function(done) {

			if (!akc_auth_token || !device_id || !akc_auth_token.length || !device_id.length)
				this.skip();

			cloud.get_user_device_types(100, false, 0, user_id, function(err, response) {
				assert.isNull(err);
				assert.isObject(response);
				console.log(JSON.stringify(response));
				done();
			}, ssl_config);
		});

	});

	testCase('#send_action()', function() {

		assertions('Send Actions to a device via Cloud', function(done) {

			if (!akc_auth_token || !device_id  || !action || !akc_auth_token.length || !device_id.length || !action.length)
				this.skip();

			assert.notInclude(cloud.send_action(device_id, action, ssl_config), "error");
			done();
		});

	});


	testCase('#send_message()', function() {

		assertions('Send Messages to Cloud from Device ', function(done) {

			if (!akc_auth_token || !device_id  || !message || !akc_auth_token.length || !device_id.length || !message.length)
				this.skip();

			assert.notInclude(cloud.send_message(device_id, message, ssl_config), "error");
			done();

		});
	});

	testCase('#add_device()', function() {

		assertions('add device', function(done) {

			if (!user_id || !device_type_id || !user_id.length || !device_type_id.length)
				this.skip();

			var response = cloud.add_device(user_id, device_type_id, "ARTIK_UT_TEST1", ssl_config);
			assert.notInclude(response, "error");
			device_id_1 = JSON.parse(response).data.id;
			done();

		});

		assertions('add device - Callback', function(done) {

			if (!user_id || !device_type_id || !user_id.length || !device_type_id.length)
				this.skip();

			cloud.add_device(user_id, device_type_id, "ARTIK_UT_TEST2", function(err, response) {
				console.log(response + err);
				assert.isNull(err);
				assert.isObject(response);
				console.log(JSON.stringify(response));
				device_id_2 = response.data.id;
				done();
			}, ssl_config);
		});
	});


	testCase('#delete_device_token()', function() {

		assertions('Delete Device Token', function(done) {

			if (!akc_auth_token || !device_id_1 || !akc_auth_token.length || !device_id_1.length)
				this.skip();

			cloud.update_device_token(device_id_1, ssl_config);

			assert.notInclude(cloud.delete_device_token(device_id_1, ssl_config), "error");
			done();

		});

		assertions('Delete Device Token - Callback', function(done) {

			if (!akc_auth_token || !device_id_2 || !akc_auth_token.length || !device_id_2.length)
				this.skip();

			cloud.update_device_token(device_id_2, ssl_config);

			cloud.delete_device_token(device_id_2, function(err, response) {
				assert.isNull(err);
				assert.isObject(response);
				console.log(JSON.stringify(response));
				done();
			}, ssl_config);
		});

	});

	testCase('#update_device_token()', function() {

		assertions('Update Device Token', function(done) {

			if (!akc_auth_token || !device_id_1 || !akc_auth_token.length || !device_id_1.length)
				this.skip();

			assert.notInclude(cloud.update_device_token(device_id_1, ssl_config), "error");
			done();

		});

		assertions('Update Device Token - Callback', function(done) {

			if (!akc_auth_token || !device_id_2 || !akc_auth_token .length || !device_id_2.length)
				this.skip();

			cloud.update_device_token(device_id_2, function(err, response) {
				assert.isNull(err);
				assert.isObject(response);
				console.log(JSON.stringify(response));
				done();
			}, ssl_config);
		});

	});

	testCase('#delete_device()', function() {

		assertions('delete device', function(done) {

			if (!device_id_1 || !device_id_1.length)
				this.skip();

			var response = cloud.delete_device(device_id_1, ssl_config);
			assert.notInclude(response, "error");
			done();

		});

		assertions('delete device - Callback', function(done) {

			if (!device_id_2 || !device_id_2.length)
				this.skip();

			cloud.delete_device(device_id_2, function(err, response) {
				console.log(response + err);
				assert.isNull(err);
				assert.isObject(response);
				console.log(JSON.stringify(response));
				done();
			}, ssl_config);
		});
	});

	testCase('#set_device_server_properties()', function () {

		assertions('set device server properties', function(done) {

			if (!device_id || !server_props || !device_id.length || !server_props.length)
				this.skip();

			var response = cloud2.set_device_server_properties(device_id, server_props, ssl_config);
			assert.notInclude(response, "error");
			done();
		});

		assertions('set device server properties - Callback', function(done) {

			if (!device_id || !server_props || !device_id.length || !server_props.length)
				this.skip();

			cloud2.set_device_server_properties(device_id, server_props, function(err, response) {
				assert.isNull(err);
				assert.isObject(response);
				console.log(JSON.stringify(response));
				done();
			}, ssl_config);
		})
	});

	testCase('#get_device_properties()', function () {

		assertions('get device properties', function(done) {

			if (!device_id || !device_id.length)
				this.skip();

			var response = cloud2.get_device_properties(device_id, timestamp, ssl_config);
			assert.isObject(JSON.parse(response).data.systemProperties);
			done();
		});

		assertions('get device properties - Callback', function(done) {

			if (!device_id || !device_id.length)
				this.skip();

			cloud2.get_device_properties(device_id, timestamp, function(err, response) {
				assert.isNull(err);
				assert.isObject(response);
				console.log(JSON.stringify(response));
				done();
			}, ssl_config);
		})
	});
});
