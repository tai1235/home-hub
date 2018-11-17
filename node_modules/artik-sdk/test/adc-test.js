/* Global Includes */
var testCase   = require('mocha').describe;
var pre        = require('mocha').before;
var preEach    = require('mocha').beforeEach;
var post       = require('mocha').after;
var postEach   = require('mocha').afterEach;
var assertions = require('mocha').it;
var assert     = require('chai').assert;
var validator  = require('validator');
var exec 	   = require('child_process').execSync;
var artik      = require('../src');


/* Test Specific Includes */
var adc;

/* Test Case Module */
testCase('ADC', function() {

	pre(function() {
		adc = artik.adc(0, "ADC0");
		adc.request();
	});

	testCase('#get_name()', function() {

		assertions('Returns the name of the ADC instance which should be ADC0', function() {
			assert.equal(adc.get_name(), "ADC0");
		});

	});

	testCase('#get_value()', function() {

		assertions('Returns the DC Voltage on ADC Pin 0', function() {
			for (i = 0; i < 5; i++) {
				var value = adc.get_value();
				console.log("ADC Value : " + value);
				assert.isAtLeast(value, 0);
			}
		});

	});

	post(function() {
		adc.release();
	});

});
