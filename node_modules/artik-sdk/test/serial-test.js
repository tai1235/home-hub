/* Global Includes */
var testCase     = require('mocha').describe;
var pre          = require('mocha').before;
var preEach      = require('mocha').beforeEach;
var post         = require('mocha').after;
var postEach     = require('mocha').afterEach;
var assertions   = require('mocha').it;
var assert       = require('chai').assert;
var validator    = require('validator');
var exec         = require('child_process').execSync;
var artik        = require('../src');

/* Test Specific Includes */
var loopback;

/* Test Case Module */
testCase('Serial', function() {

	pre(function() {
		const name = artik.get_platform_name();
		if(name == 'ARTIK 520') {
			console.log("Running SERIAL test on ARTIK 520");
			const a5 = require('../src/platform/artik520');
			port = a5.ARTIK_A5_SERIAL.SCOM.XSCOM4;
		} else if(name == 'ARTIK 1020') {
			console.log("Running SERIAL test on ARTIK 1020");
			const a10 = require('../src/platform/artik1020');
			port = a10.ARTIK_A10_SERIAL.SCOM.XSCOM2;
		} else if(name == 'ARTIK 710') {
			console.log("Running SERIAL test on ARTIK 710");
			const a7 = require('../src/platform/artik710');
			port = a7.ARTIK_A710_SERIAL.UART.UART0;
		} else if(name == 'ARTIK 530') {
			console.log("Running SERIAL test on ARTIK 530");
			const a530 = require('../src/platform/artik530');
			port = a530.ARTIK_A530_SERIAL.UART.UART4;
		} else if(name == 'ARTIK 305') {
			console.log("Running SERIAL test on ARTIK 305");
			const a305 = require('../src/platform/artik305');
			port = a305.ARTIK_A305_SERIAL.UART.UART2;
		} else if(name == 'Eagleye530') {
			console.log("Running SERIAL test on Eagleye530");
			const ee530 = require('../src/platform/eagleye530');
			port = ee530.ARTIK_EAGLEYE530_SERIAL.UART.UART4;
		}

		loopback = new artik.serial(port, "serial-loopback", 115200, "none", 8,
					1, "none");

		loopback.request();

	});

	testCase('#write(), read()', function() {

		assertions('Write the data to serial loopback port and read it back', function(done) {

			this.timeout(5000);

			console.log('Starting Loopback Test...Make sure you have connected the Tx and Rx of the Serial Port with a wire');
			var tx_buf = new Buffer('aabbccddeeff');
			var rx_buf = null;

			/* Transfer Data */
			loopback.on('read', function(data) {
				if (rx_buf == null) {
					rx_buf = new Buffer(data);
				}

				if (rx_buf.length < tx_buf.length) {
					rx_buf = Buffer.concat(rx_buf, data);
				}

				if (rx_buf.length >= tx_buf.length) {
					assert.equal(tx_buf.length, rx_buf.length);
					assert.equal(tx_buf.equals(rx_buf), true);
					done();
				}
			});

			console.log("Sending " + tx_buf.length + " bytes on the serial port");
			var written = loopback.write(tx_buf);
			console.log("Wrote " + written + " bytes");

		});

	});

    testCase('#get_*(), #set_*()', function() {
        assertions('Get the value that are passed to the constructor', function(done) {
        	assert.equal(loopback.get_baudrate(), 115200);
			assert.equal(loopback.get_parity(), 'none');
			assert.equal(loopback.get_data_bits(), 8);
			assert.equal(loopback.get_stop_bits(), 1);
			assert.equal(loopback.get_flowctrl(), 'none');
			done();
		});

		assertions('Set new value and get it', function(done) {
			loopback.set_baudrate(9600);
			assert.equal(loopback.get_baudrate(), 9600);

			loopback.set_parity('odd');
			assert.equal(loopback.get_parity(), 'odd');

			loopback.set_data_bits(7);
			assert.equal(loopback.get_data_bits(), 7);

			loopback.set_stop_bits(2);
			assert.equal(loopback.get_stop_bits(), 2);

			loopback.set_flowctrl('soft');
      		assert.equal(loopback.get_flowctrl(), 'soft');
			done();
		});

		assertions('Set invalid value', function done() {
			assert.throws(function() { loopback.set_baudrate(0) }, TypeError, 0);
			assert.throws(function() { loopback.set_parity('noodd') }, TypeError, 'noodd');
			assert.throws(function() { loopback.set_data_bits('8bit') }, TypeError, 'nan');
			assert.throws(function() { loopback.set_stop_bits('1bit') }, TypeError, 'nan');
			assert.throws(function() { loopback.set_flowctrl(1) }, TypeError, 1);
		});
    });

	post(function() {
		//loopback.release();
	});

});
