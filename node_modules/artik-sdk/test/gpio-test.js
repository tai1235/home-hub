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
var runManualTests = parseInt(process.env.RUN_MANUAL_TESTS);

/* Test Specific Includes */
var button, red, green, blue, led400, led401, sw403, sw404;

/* Test Case Module */
testCase('GPIO', function() {

	pre(function() {
		const name = artik.get_platform_name();

		if(name == 'ARTIK 520') {
			console.log('Running GPIO test on ARTIK 520');
			const a5 = require('../src/platform/artik520');
			red      = new artik.gpio(a5.ARTIK_A5_GPIO_XEINT0, 'red', 'out', 'none', 0);
			green    = new artik.gpio(a5.ARTIK_A5_GPIO_XEINT1, 'green', 'out', 'none', 0);
			blue     = new artik.gpio(a5.ARTIK_A5_GPIO_XEINT2, 'blue', 'out', 'none', 0);
			button   = new artik.gpio(a5.ARTIK_A5_GPIO_XEINT3, 'button', 'in', 'both', 0);
		} else if(name == 'ARTIK 1020') {
			console.log('Running GPIO test on ARTIK 1020');
			const a10 = require('../src/platform/artik1020');
			red       = new artik.gpio(a10.ARTIK_A10_GPIO_XEINT0, 'red', 'out', 'none', 0);
			green     = new artik.gpio(a10.ARTIK_A10_GPIO_XEINT1, 'green', 'out', 'none', 0);
			blue      = new artik.gpio(a10.ARTIK_A10_GPIO_XEINT2, 'blue', 'out', 'none', 0);
			button    = new artik.gpio(a10.ARTIK_A10_GPIO_XEINT3, 'button', 'in', 'both', 0);
		} else if(name == 'ARTIK 710') {
			console.log('Running GPIO test on ARTIK 710');
			const a7 = require('../src/platform/artik710');
			led400 	 = new artik.gpio(a7.ARTIK_A710_GPIO_LED400, 'led400', 'out', 'none', 0);
			led401 	 = new artik.gpio(a7.ARTIK_A710_GPIO_LED401, 'led401', 'out', 'none', 0);
			sw403 	 = new artik.gpio(a7.ARTIK_A710_GPIO_SW403, 'sw403', 'in', 'both', 0);
			sw404 	 = new artik.gpio(a7.ARTIK_A710_GPIO_SW404, 'sw404', 'in', 'both', 0);
		} else if(name == 'ARTIK 530') {
			console.log('Running GPIO test on ARTIK 530');
			const a530 = require('../src/platform/artik530');
			led400 	   = new artik.gpio(a530.ARTIK_A530_GPIO_LED400, 'led400', 'out', 'none', 0);
			led401 	   = new artik.gpio(a530.ARTIK_A530_GPIO_LED401, 'led401', 'out', 'none', 0);
			sw403 	   = new artik.gpio(a530.ARTIK_A530_GPIO_SW403, 'sw403', 'in', 'both', 0);
			sw404 	   = new artik.gpio(a530.ARTIK_A530_GPIO_SW404, 'sw404', 'in', 'both', 0);
		} else if(name == 'ARTIK 305') {
			console.log('Running GPIO test on ARTIK 305');
			const a305 = require('../src/platform/artik305');
			led400 	   = new artik.gpio(a305.ARTIK_A305_GPIO_LED400, 'led400', 'out', 'none', 0);
			led401 	   = new artik.gpio(a305.ARTIK_A305_GPIO_LED401, 'led401', 'out', 'none', 0);
			sw403 	   = new artik.gpio(a305.ARTIK_A305_GPIO_SW403, 'sw403', 'in', 'both', 0);
			sw404 	   = new artik.gpio(a305.ARTIK_A305_GPIO_SW404, 'sw404', 'in', 'both', 0);
		} else if(name == 'Eagleye530') {
			console.log('Running GPIO test on Eagleye530');
			const ee530 = require('../src/platform/eagleye530');
			led400 	   = new artik.gpio(ee530.ARTIK_EAGLEYE530_GPIO_LED1, 'led1', 'out', 'none', 0);
			led401 	   = new artik.gpio(ee530.ARTIK_EAGLEYE530_GPIO_LED2, 'led2', 'out', 'none', 0);
			sw403 	   = new artik.gpio(ee530.ARTIK_EAGLEYE530_GPIO_SW1, 'sw1', 'in', 'both', 0);
			sw404 	   = new artik.gpio(ee530.ARTIK_EAGLEYE530_GPIO_SW2, 'sw2', 'in', 'both', 0);
		}

		if (name == 'ARTIK 520' || name == 'ARTIK 1020') {
			red.request();
			green.request();
			blue.request();
			red.write(0);
			blue.write(0);
			green.write(0);
		}

		if (name == 'ARTIK 530' || name == 'ARTIK 710' || name == 'ARTIK 305' || name == 'Eagleye530') {
			led400.request();
			led401.request();
			led400.write(0);
			led401.write(0);
		}

	});

	testCase('#write #read', function() {

		assertions('Sets the GPIO Level', function() {

			const name = artik.get_platform_name();

			if (name == 'ARTIK 530' || name == 'ARTIK 710' || name == 'ARTIK 305' || name == 'Eagleye530')
				this.skip();

			red.write(1);
			blue.write(1);
			green.write(1);

			try {
				red.read();
			} catch(e) {
				assert.equal(e, "TypeError: Access denied");
			}

			try {
				green.read();
			} catch(e) {
				assert.equal(e, "TypeError: Access denied");
			}

			try {
				blue.read();
			} catch(e) {
				assert.equal(e, "TypeError: Access denied");
			}

		});

		assertions('Sets the LED Level', function() {

			const name = artik.get_platform_name();

			if (name != 'ARTIK 530' && name != 'ARTIK 710' && name != 'ARTIK 305' && name != 'Eagleye530')
				this.skip();

			led400.write(1);
			led401.write(1);

			try {
				led400.read();
			} catch(e) {
				assert.equal(e, "TypeError: Access denied");
			}

			try {
				led401.read();
			} catch(e) {
				assert.equal(e, "TypeError: Access denied");
			}

		});

	});

    testCase('#button', function() {

        assertions('Get Button Press Event', function(done) {

        const name = artik.get_platform_name();

		if (name == 'ARTIK 530' || name == 'ARTIK 710' || name == 'ARTIK 305' || name == 'Eagleye530')
			this.skip();

		this.timeout(10000);
		console.log("Please press the button within 10 seconds ");
		button.on('changed', function(val) {
			console.log("Button state: " + val);
			done();
		});
		button.request();

        });

    });

    testCase('#sw403', function() {

	assertions('Get SW403 Button Press Event', function(done) {

		const name = artik.get_platform_name();

		if (!runManualTests || (name != 'ARTIK 530' && name != 'ARTIK 710' && name != 'ARTIK 305' && name != 'Eagleye530'))
			this.skip();

		this.timeout(10000);
		console.log("Please press the SW403 button within 10 seconds ");
		sw403.on('changed', function(val) {
			console.log("SW403 Button state: " + val);
			if (val == 1)
				done();
		});
		sw403.request();

        });

    });

    testCase('#sw404', function() {

	assertions('Get SW404 Button Press Event', function(done) {

		const name = artik.get_platform_name();

		if (!runManualTests || (name != 'ARTIK 530' && name != 'ARTIK 710' && name != 'ARTIK 305' && name != 'Eagleye530'))
			this.skip();

		this.timeout(10000);
		console.log("Please press the SW404 button within 10 seconds ");
		sw404.on('changed', function(val) {
			console.log("SW404 Button state: " + val);
			if (val == 1)
				done();
		});
		sw404.request();

        });

    });

	post(function() {
		const name = artik.get_platform_name();

		if (name != 'ARTIK 530' && name != 'ARTIK 710' && name != 'ARTIK 305' && name != 'Eagleye530') {
			red.write(0);
			blue.write(0);
			green.write(0);

			red.release();
			green.release();
			blue.release();
			button.release();
		}

		if (name == 'ARTIK 530' || name == 'ARTIK 710' || name == 'ARTIK 305' || name == 'Eagleye530') {
			led400.write(0);
			led401.write(0);

			led400.release();
			led401.release();

			sw403.release();
			sw404.release();
		}
	});
});
