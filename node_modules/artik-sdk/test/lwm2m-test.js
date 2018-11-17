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
var artik      = require('../src');

/* Test Specific Includes */
var lwm2m = new artik.lwm2m();

var server_id = 123;
var server_uri = 'coaps://coaps-api.artik.cloud:5686';
var lifetime = 30;
var connect_timeout = 1000;
var dtls_psk_id = process.env.LWM2M_DEVICE_ID;
var dtls_psk_key = process.env.LWM2M_DEVICE_TOKEN;

var objects = {
    device: {
        manufacturer: 'Samsung',
        model: 'ARTIK',
        serial: '1234567890',
        fwVersion: '1.0',
        hwVersion: '1.0',
        swVersion: '1.0',
        deviceType: 'Hub',
        powerSource: 0,
        powerVoltage: 5000,
        powerCurrent: 1500,
        batteryLevel: 100,
        memoryTotal: 1000000,
        memoryFree: 200000,
        timeZone: 'Europe/Paris',
        utcOffset: '+01:00',
        bindingModes: 'U'
    },
    conn_monitoring: {
	netbearer : 0,
	avalnetbearer : 1,
	signalstrength : 12,
	linkquality : 1,
	ipaddr : '192.168.1.34',
	ipaddr2 : '192.168.1.225',
	routeaddr : '1.1.1.1',
	routeaddr2 : '1.6.7.8',
	linkutilization : 1,
	apn : 'SAMI2_5G',
	cellid : 386,
	smnc : 189,
	smcc : 33
    }
};


/* Test Case Module */
testCase('Lwm2m', function() {

	pre(function() {
		lwm2m.on('started', function() {
		});
	});

	testCase('client_connect()', function() {
		assertions('Connect to the LWM2M Server', function(done) {

			if (!dtls_psk_id || !dtls_psk_key || !dtls_psk_id.length || !dtls_psk_key.length)
				this.skip();

			lwm2m.client_request(server_id, server_uri, dtls_psk_id, lifetime, connect_timeout,
								 objects, dtls_psk_id, dtls_psk_key);
			lwm2m.client_connect();
			done();
		});
	});

	testCase('serialize_tlv_int()', function() {
		assertions('Serialize array of integer', function(done) {
		    var buff_int_serialized = lwm2m.serialize_tlv_int([0, 1]);
		    console.log("Integer buffer serialized :")
		    for (var i = 0; i < buff_int_serialized.length; ++i )
		    {
			process.stdout.write(" " + buff_int_serialized.readUInt8(i));
		    }
		    process.stdout.write("\n");
		    var resource_uri = "/3/0/11";
		    lwm2m.client_write_resource(resource_uri, buff_int_serialized);
		    var reg = lwm2m.client_read_resource(resource_uri);
		    console.log("response :"+reg)
		    assert.equal(reg.toString("utf-8", 0, 7), "0=0,1=1");
		    done();
		});
	});

	testCase('serialize_tlv_string()', function() {
		assertions('Serialize array of string', function(done) {
		    var buff_str_serialized = lwm2m.serialize_tlv_string(["192.168.1.27", "192.168.9.9"]);
		    console.log("String buffer serialized :")
		    for (var i = 0; i < buff_str_serialized.length; ++i )
		    {
			process.stdout.write(" " + buff_str_serialized.readUInt8(i));
		    }
		    process.stdout.write("\n");
		    var resource_uri = "/4/0/4";
		    lwm2m.client_write_resource(resource_uri, buff_str_serialized);
		    var reg = lwm2m.client_read_resource(resource_uri);
		    console.log("response :"+reg)
		    assert.equal(reg.toString("utf-8", 0, 7), "0=D,1=D");
		    done();
		});
	});

	testCase('client_read_resource()', function() {
		assertions('Reads the resource', function(done) {

			if (!dtls_psk_id || !dtls_psk_key || !dtls_psk_id.length || !dtls_psk_key.length)
				this.skip();

			var buf = lwm2m.client_read_resource("/3/0/0");
			assert.equal(buf, "Samsung");
			done();
		});
	});

	testCase('client_write_resource()', function() {
	    assertions('Write the resource', function(done) {

		if (!dtls_psk_id || !dtls_psk_key || !dtls_psk_id.length || !dtls_psk_key.length)
		    this.skip();

		var resource_uri = "/3/0/3";
		var buf = new Buffer("2.0");
		lwm2m.client_write_resource(resource_uri, buf);
		var reg = lwm2m.client_read_resource(resource_uri);
		assert.equal(reg, "2.0");
		done();
	    });
	});


	post(function() {
	});

});
