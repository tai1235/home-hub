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
var spi;

/* Test Case Module */
testCase('SPI', function() {

	pre(function() {
		const name = artik.get_platform_name();

		if(name == 'ARTIK 520') {
			console.log('Running SPI test on ARTIK 520');
			const a5 = require('../src/platform/artik520');
			spi = artik.spi(a5.ARTIK_A5_SPI.BUS.BUS1,
								a5.ARTIK_A5_SPI.CS.CS0,
								a5.ARTIK_A5_SPI.MODE.MODE0,
								a5.ARTIK_A5_SPI.BITS.BITS8,
								500000);
		} else if(name == 'ARTIK 1020') {
			console.log('Running SPI test on ARTIK 1020');
			const a10 = require('../src/platform/artik1020');
			spi = artik.spi(a10.ARTIK_A10_SPI.BUS.BUS1,
								a10.ARTIK_A10_SPI.CS.CS0,
								a10.ARTIK_A10_SPI.MODE.MODE0,
								a10.ARTIK_A10_SPI.BITS.BITS8,
								500000);
		} else if(name == 'ARTIK 710') {
			console.log('Running SPI test on ARTIK 710');
			const a7 = require('../src/platform/artik710.js');
			spi = artik.spi(a7.ARTIK_A710_SPI.BUS.BUS0,
								a7.ARTIK_A710_SPI.CS.CS0,
								a7.ARTIK_A710_SPI.MODE.MODE0,
								a7.ARTIK_A710_SPI.BITS.BITS8,
								500000);
		} else if(name == 'ARTIK 530') {
			console.log('Running SPI test on ARTIK 530');
			const a530 = require('../src/platform/artik530.js');
			spi = artik.spi(a530.ARTIK_A530_SPI.BUS.BUS2,
								a530.ARTIK_A530_SPI.CS.CS0,
								a530.ARTIK_A530_SPI.MODE.MODE0,
								a530.ARTIK_A530_SPI.BITS.BITS8,
								500000);
		} else if(name == 'ARTIK 305') {
			console.log('Running SPI test on ARTIK 305');
			const a305 = require('../src/platform/artik305.js');
			spi = artik.spi(a305.ARTIK_A305_SPI.BUS.BUS0,
								a305.ARTIK_A305_SPI.CS.CS0,
								a305.ARTIK_A305_SPI.MODE.MODE0,
								a305.ARTIK_A305_SPI.BITS.BITS8,
								500000);
		} else if(name == 'Eagleye530') {
			console.log('Running SPI test on Eagleye530');
			const ee530 = require('../src/platform/eagleye530.js');
			spi = artik.spi(ee530.ARTIK_EAGLEYE530_SPI.BUS.BUS0,
							ee530.ARTIK_EAGLEYE530_SPI.CS.CS0,
							ee530.ARTIK_EAGLEYE530_SPI.MODE.MODE0,
							ee530.ARTIK_EAGLEYE530_SPI.BITS.BITS8,
							500000);
		}

		spi.request();
	});

	testCase('#read_write()', function() {

		assertions('Write the data to spi loopback port and read it back', function() {

			console.log('Starting Loopback Test...Make sure you have connected MOSI and MISO with a wire');
			var tx_buf = new Buffer([0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9] , 'hex');
			var rx_buf = new Buffer([0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0],  'hex');

			/* Transfer Data */
			console.log("Sending " + tx_buf.length + " bytes on the spi bus");
			rx_buf = spi.read_write(tx_buf, tx_buf.length);
			console.log("Received " + rx_buf.length + " bytes on the spi bus");
			assert.equal(tx_buf.equals(rx_buf), true);
		});

	});

	post(function() {
		spi.release();
	});

});
