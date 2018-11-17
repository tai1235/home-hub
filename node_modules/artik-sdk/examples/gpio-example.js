const artik = require('../src');
var button, red, green, blue, led400, led401, sw403, sw404;

const name = artik.get_platform_name();

if(name == 'ARTIK 520') {
	console.log('Running GPIO test on ARTIK 520');
	const a5 = require('../src/platform/artik520');
	red = new artik.gpio(a5.ARTIK_A5_GPIO_XEINT0, 'red', 'out', 'none', 0);
	green = new artik.gpio(a5.ARTIK_A5_GPIO_XEINT1, 'green', 'out', 'none', 0);
	blue = new artik.gpio(a5.ARTIK_A5_GPIO_XEINT2, 'blue', 'out', 'none', 0);
	button = new artik.gpio(a5.ARTIK_A5_GPIO_XEINT3, 'button', 'in' , 'both', 0);
} else if(name == 'ARTIK 1020') {
	console.log('Running GPIO test on ARTIK 1020');
	const a10 = require('../src/platform/artik1020');
	red = new artik.gpio(a10.ARTIK_A10_GPIO_XEINT0, 'red', 'out', 'none', 0);
	green = new artik.gpio(a10.ARTIK_A10_GPIO_XEINT1, 'green', 'out', 'none', 0);
	blue = new artik.gpio(a10.ARTIK_A10_GPIO_XEINT2, 'blue', 'out', 'none', 0);
	button = new artik.gpio(a10.ARTIK_A10_GPIO_XEINT3, 'button', 'in' , 'both', 0);
} else if(name == 'ARTIK 710') {
	console.log('Running GPIO test on ARTIK 710');
	const a7 = require('../src/platform/artik710');
	red = new artik.gpio(a7.ARTIK_A710_GPIO_GPIO0, 'red', 'out', 'none', 0);
	green = new artik.gpio(a7.ARTIK_A710_GPIO_GPIO1, 'green', 'out', 'none', 0);
	blue = new artik.gpio(a7.ARTIK_A710_GPIO_GPIO2, 'blue', 'out', 'none', 0);
	led400 = new artik.gpio(a7.ARTIK_A710_GPIO_LED400, 'led400', 'out', 'none', 0);
	led401 = new artik.gpio(a7.ARTIK_A710_GPIO_LED401, 'led401', 'out', 'none', 0);
	sw403 = new artik.gpio(a7.ARTIK_A710_GPIO_SW403, 'sw403', 'in', 'both', 0);
	sw404 = new artik.gpio(a7.ARTIK_A710_GPIO_SW404, 'sw404', 'in', 'both', 0);
	button = new artik.gpio(a7.ARTIK_A710_GPIO_GPIO4, 'button', 'in' , 'both', 0);
} else if(name == 'ARTIK 530') {
	console.log('Running GPIO test on ARTIK 530');
	const a530 = require('../src/platform/artik530');
	red = new artik.gpio(a530.ARTIK_A530_GPIO_GPIO0, 'red', 'out', 'none', 0);
	green = new artik.gpio(a530.ARTIK_A530_GPIO_GPIO1, 'green', 'out', 'none', 0);
	blue = new artik.gpio(a530.ARTIK_A530_GPIO_GPIO2, 'blue', 'out', 'none', 0);
	led400 = new artik.gpio(a530.ARTIK_A530_GPIO_LED400, 'led400', 'out', 'none', 0);
	led401 = new artik.gpio(a530.ARTIK_A530_GPIO_LED401, 'led401', 'out', 'none', 0);
	sw403 = new artik.gpio(a530.ARTIK_A530_GPIO_SW403, 'sw403', 'in', 'both', 0);
	sw404 = new artik.gpio(a530.ARTIK_A530_GPIO_SW404, 'sw404', 'in', 'both', 0);
	button = new artik.gpio(a530.ARTIK_A530_GPIO_GPIO4, 'button', 'in' , 'both', 0);
} else if (name == 'ARTIK 305') {
	console.log('Running GPIO test on ARTIK 305');
	const a305 = require('../src/platform/artik305');
	red = new artik.gpio(a305.ARTIK_A305_GPIO_GPIO0, 'red', 'out', 'none', 0);
	green = new artik.gpio(a305.ARTIK_A305_GPIO_GPIO1, 'green', 'out', 'none', 0);
	blue = new artik.gpio(a305.ARTIK_A305_GPIO_GPIO2, 'blue', 'out', 'none', 0);
	led400 = new artik.gpio(a305.ARTIK_A305_GPIO_LED400, 'led400', 'out', 'none', 0);
	led401 = new artik.gpio(a305.ARTIK_A305_GPIO_LED401, 'led401', 'out', 'none', 0);
	sw403 = new artik.gpio(a305.ARTIK_A305_GPIO_SW403, 'sw403', 'in', 'both', 0);
	sw404 = new artik.gpio(a305.ARTIK_A305_GPIO_SW404, 'sw404', 'in', 'both', 0);
	button = new artik.gpio(a305.ARTIK_A305_GPIO_GPIO4, 'button', 'in', 'both', 0);
} else if(name == 'Eagleye530') {
	console.log('Running GPIO test on Eagleye530');
	const ee530 = require('../src/platform/eagleye530');
	red = new artik.gpio(ee530.ARTIK_EAGLEYE530_GPIO_GPIO0, 'red', 'out', 'none', 0);
	green = new artik.gpio(ee530.ARTIK_EAGLEYE530_GPIO_GPIO1, 'green', 'out', 'none', 0);
	blue = new artik.gpio(ee530.ARTIK_EAGLEYE530_GPIO_GPIO2, 'blue', 'out', 'none', 0);
	led400 = new artik.gpio(ee530.ARTIK_EAGLEYE530_GPIO_LED1, 'led1', 'out', 'none', 0);
	led401 = new artik.gpio(ee530.ARTIK_EAGLEYE530_GPIO_LED2, 'led2', 'out', 'none', 0);
	sw403 = new artik.gpio(ee530.ARTIK_EAGLEYE530_GPIO_SW1, 'sw1', 'in', 'both', 0);
	sw404 = new artik.gpio(ee530.ARTIK_EAGLEYE530_GPIO_SW2, 'sw2', 'in', 'both', 0);
	button = new artik.gpio(ee530.ARTIK_EAGLEYE530_GPIO_GPIO4, 'button', 'in' , 'both', 0);
} else {
    console.log('Unrecognized platform');
    process.exit(-1);
}

button.on('changed', function(val) {
	console.log("Button state: " + val);
});

button.request();

if (name == 'ARTIK 530' || name == 'ARTIK 710' || name == 'ARTIK 305' || name == 'Eagleye530') {
	led400.request();
	led400.write(1);
	led401.request();
	led401.write(1);

	sw403.on('changed', function(val) {
		console.log("sw403 state: " + val);
	});

	sw403.request();

	sw404.on('changed', function(val) {
		console.log("sw404 state: " + val);
	});

	sw404.request();
}

var color = 0;

console.log('Launch periodic interval');

setInterval(function () {

	console.log('Setting color');

	red.request();
	red.write(color & 1);
	red.release();

	green.request();
	green.write(color & 2);
	green.release();

	blue.request();
	blue.write(color & 4);
	blue.release();

	color = (color + 1) % 8;

},1000);

process.on('SIGINT', function () {
	console.log('exiting test');
	button.release();

	if (name == 'ARTIK 530' || name == 'ARTIK 710' || name == 'ARTIK 305'){
		led400.write(0);
		led400.release();
		led401.write(0);
		led401.release();

		sw403.release();
		sw404.release();
	}

	process.exit(0);
});
