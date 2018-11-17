const artik = require('../src');

const name = artik.get_platform_name();

if(name == 'ARTIK 520') {
	console.log('Running SPI test on ARTIK 520');
	const a5 = require('../src/platform/artik520');
	var spi = artik.spi(a5.ARTIK_A5_SPI.BUS.BUS1,
					  a5.ARTIK_A5_SPI.CS.CS0,
					  a5.ARTIK_A5_SPI.MODE.MODE0,
					  a5.ARTIK_A5_SPI.BITS.BITS8,
					  500000);
} else if(name == 'ARTIK 1020') {
	console.log('Running SPI test on ARTIK 1020');
	const a10 = require('../src/platform/artik1020');
	var spi = artik.spi(a10.ARTIK_A10_SPI.BUS.BUS1,
						  a10.ARTIK_A10_SPI.CS.CS0,
						  a10.ARTIK_A10_SPI.MODE.MODE0,
						  a10.ARTIK_A10_SPI.BITS.BITS8,
						  500000);
} else if(name == 'ARTIK 710') {
	console.log('Running SPI test on ARTIK 710');
	const a7 = require('../src/platform/artik710.js');
	var spi = artik.spi(a7.ARTIK_A710_SPI.BUS.BUS0,
						  a7.ARTIK_A710_SPI.CS.CS0,
						  a7.ARTIK_A710_SPI.MODE.MODE0,
						  a7.ARTIK_A710_SPI.BITS.BITS8,
						  500000);
} else if(name == 'ARTIK 530') {
	console.log('Running SPI test on ARTIK 530');
	const a530 = require('../src/platform/artik530.js');
	var spi = artik.spi(a530.ARTIK_A530_SPI.BUS.BUS0,
						  a530.ARTIK_A530_SPI.CS.CS0,
						  a530.ARTIK_A530_SPI.MODE.MODE0,
						  a530.ARTIK_A530_SPI.BITS.BITS8,
						  500000);
} else if(name == 'ARTIK 305') {
	console.log('Running SPI test on ARTIK 305');
	const a305 = require('../src/platform/artik305.js');
	var spi = artik.spi(a305.ARTIK_A305_SPI.BUS.BUS0,
						  a305.ARTIK_A305_SPI.CS.CS0,
						  a305.ARTIK_A305_SPI.MODE.MODE0,
						  a305.ARTIK_A305_SPI.BITS.BITS8,
						  500000);
} else if(name == 'Eagleye530') {
	console.log('Running SPI test on Eagleye530');
	const ee530 = require('../src/platform/eagleye530.js');
	var spi = artik.spi(ee530.ARTIK_EAGLEYE530_SPI.BUS.BUS0,
						ee530.ARTIK_EAGLEYE530_SPI.CS.CS0,
						ee530.ARTIK_EAGLEYE530_SPI.MODE.MODE0,
						ee530.ARTIK_EAGLEYE530_SPI.BITS.BITS8,
						500000);
} else {
	console.log('Unrecognized platform');
	process.exit(-1);
}

if (spi.request()) {
	console.log('Failed to request spi');
} else {

	console.log('Starting Loopback Test...Make sure you have connected MOSI and MISO with a wire');
	var tx_buf = new Buffer([0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9] , 'hex');

	/* Transfer Data */
	console.log("Sending " + tx_buf.length + " bytes on the spi bus");
	var rx_buf = spi.read_write(tx_buf);

	/* Compare the received data */
	if (tx_buf.equals(rx_buf))
		console.log("SPI Test Passed");
	else
		console.log("SPI Test Failed");

	spi.release();
}
