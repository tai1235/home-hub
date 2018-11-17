const artik = require('../src');

const name = artik.get_platform_name();

if(name == 'ARTIK 520') {
    console.log('Running PWM test on ARTIK 520');
    const a520 = require('../src/platform/artik520');
    var pwm = artik.pwm(a520.ARTIK_A5_PWM.PWMIO.XPWMIO1, "pwm-test", 400000, a520.ARTIK_A5_PWM.POLR.NORMAL, 200000);
} else if(name == 'ARTIK 1020') {
    console.log('Running PWM test on ARTIK 1020');
    const a1020 = require('../src/platform/artik1020');
    var pwm = artik.pwm(a1020.ARTIK_A10_PWM.PWMIO.XPWMIO1, "pwm-test", 400000, a1020.ARTIK_A10_PWM.POLR.NORMAL, 200000);
} else if(name == 'ARTIK 710') {
    console.log('Running PWM test on ARTIK 710');
    const a710 = require('../src/platform/artik710');
    var pwm = artik.pwm(a710.ARTIK_A710_PWM.PWMIO.XPWMIO0, "pwm-test", 400000, a710.ARTIK_A710_PWM.POLR.NORMAL, 200000);
} else if(name == 'ARTIK 530') {
    console.log('Running PWM test on ARTIK 530');
    const a530 = require('../src/platform/artik530');
    var pwm = artik.pwm(a530.ARTIK_A530_PWM.PWMIO.XPWMIO0, "pwm-test", 400000, a530.ARTIK_A530_PWM.POLR.NORMAL, 200000);
} else if(name == 'ARTIK 305') {
    console.log('Running PWM test on ARTIK 305');
    const a305 = require('../src/platform/artik305');
    var pwm = artik.pwm(a305.ARTIK_A305_PWM.PWMIO.XPWMIO0, "pwm-test", 400000, a305.ARTIK_A305_PWM.POLR.NORMAL, 200000);
} else if(name == 'Eagleye530') {
    console.log('Running PWM test on Eagleye530');
    const ee530 = require('../src/platform/eagleye530');
    var pwm = artik.pwm(ee530.ARTIK_EAGLEYE530_PWM.PWMIO.PWMIO0, "pwm-test", 400000, ee530.ARTIK_EAGLEYE530_PWM.POLR.NORMAL, 200000);
} else {
    console.log('Unrecognized platform');
    process.exit(-1);
}

var dc = 150000;

pwm.request();
console.log("[%s] :  request.", pwm.get_name());

var intervalFunc = setInterval(function() {
    if (dc >= 400000)
        dc = 150000;
    console.log("[%s] :  Change duty cycle .", pwm.get_name());
    pwm.set_duty_cycle(dc);
    dc += 50000;
}, 800);


process.on('SIGINT', function () {
    pwm.release();
    console.log("[%s] :  release.", pwm.get_name());
    clearInterval(intervalFunc);
    process.kill(process.pid);
});

