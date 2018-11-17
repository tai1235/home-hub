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
var pwm;
var dc = 150000;

/* Test Case Module */
testCase('PWM', function() {

	pre(function() {
		const name = artik.get_platform_name();

		if(name == 'ARTIK 520') {
			console.log('Running PWM test on ARTIK 520');
			const a5 = require('../src/platform/artik520');
			pwm = artik.pwm(a5.ARTIK_A5_PWM.PWMIO.XPWMIO1, "pwm-test", 400000, a5.ARTIK_A5_PWM.POLR.NORMAL, 200000);
		} else if(name == 'ARTIK 1020') {
			console.log('Running PWM test on ARTIK 1020');
			const a10 = require('../src/platform/artik1020');
			pwm = artik.pwm(a10.ARTIK_A10_PWM.PWMIO.XPWMIO1, "pwm-test", 400000, a10.ARTIK_A10_PWM.POLR.NORMAL, 200000);
		} else if(name == 'ARTIK 710') {
			console.log('Running PWM test on ARTIK 710');
			const a7 = require('../src/platform/artik710');
			pwm = artik.pwm(a7.ARTIK_A710_PWM.PWMIO.XPWMIO1, "pwm-test", 400000, a7.ARTIK_A710_PWM.POLR.NORMAL, 200000);
		} else if(name == 'ARTIK 530') {
			console.log('Running PWM test on ARTIK 530');
			const a530 = require('../src/platform/artik530');
			pwm = artik.pwm(a530.ARTIK_A530_PWM.PWMIO.XPWMIO0, "pwm-test", 400000, a530.ARTIK_A530_PWM.POLR.NORMAL, 200000);
		} else if(name == 'ARTIK 305') {
			console.log('Running PWM test on ARTIK 305');
			const a305 = require('../src/platform/artik305');
			pwm = artik.pwm(a305.ARTIK_A305_PWM.PWMIO.XPWMIO0, "pwm-test", 400000, a530.ARTIK_A305_PWM.POLR.NORMAL, 200000);
		} else if(name == 'Eagleye530') {
			console.log('Running PWM test on Eagleye530');
			const ee530 = require('../src/platform/eagleye530');
			pwm = artik.pwm(ee530.ARTIK_EAGLEYE530_PWM.PWMIO.XPWMIO0, "pwm-test", 400000, ee530.ARTIK_EAGLEYE530_PWM.POLR.NORMAL, 200000);
		}
		pwm.request();
	});

	testCase('#set_duty_cycle()', function() {

		assertions('Set the duty cylcle on the PWM port', function() {
			for (dc = 150000; dc < 400000; dc++)
				pwm.set_duty_cycle(dc);
		});

	});

	post(function() {
		pwm.release();
	});

});
