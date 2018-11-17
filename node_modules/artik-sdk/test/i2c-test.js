/* Global Includes */
var testCase       = require('mocha').describe;
var pre            = require('mocha').before;
var preEach        = require('mocha').beforeEach;
var post           = require('mocha').after;
var postEach       = require('mocha').afterEach;
var assertions     = require('mocha').it;
var assert         = require('chai').assert;
var validator      = require('validator');
var exec           = require('child_process').execSync;
var artik          = require('../src');
var runManualTests = process.env.RUN_MANUAL_TESTS;


/* Test Specific Includes */
var cw2015;

/* Test Case Module */
testCase('I2C', function() {

	pre(function() {
		const name = artik.get_platform_name();

		if(name == 'ARTIK 520') {
			console.log('Running I2C test on ARTIK 520');
		} else if(name == 'ARTIK 1020') {
			console.log('Running I2C test on ARTIK 1020');
			cw2015 = artik.i2c(0, 2000, '8', 0x62);
		} else if(name == 'ARTIK 710') {
			console.log('Running I2C test on ARTIK 710');
			cw2015 = artik.i2c(8, 2000, '8', 0x62);
		} else if(name == 'ARTIK 530') {
			console.log('Running I2C test on ARTIK 530');
			cw2015 = artik.i2c(8, 2000, '8', 0x62);
		} else if(name == 'ARTIK 305') {
			console.log('Running I2C test on ARTIK 305');
			cw2015 = artik.i2c(8, 2000, '8', 0x62);
		}

		cw2015.request();

	});

	testCase('#read_register', function() {

		assertions('Read Register Values ', function() {
			var version = Buffer(cw2015.read_register(0, 1)).toString('hex');
			console.log(version);
			assert.isNotNull(version);
		});

	});

	testCase('#write_register', function() {

		assertions('Write Register Values ', function() {
			var reg = cw2015.read_register(8, 1);
			console.log('Config: 0x' + Buffer(reg).toString('hex'));
			reg = new Buffer([0xff], 'hex');
			console.log('Writing 0x' + Buffer(reg).toString('hex') + ' to config register');
			cw2015.write_register(8, reg);
			reg = cw2015.read_register(8, 1);
			console.log('Config: 0x' + Buffer(reg).toString('hex'));
			assert.isNotNull(reg);
		});

	});

	post(function() {
		cw2015.release();
	});

});
