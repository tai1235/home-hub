/*
 * This test only works if the CW2015 Linux driver is unbound first:
 * artik520 : $ echo 1-0062 > /sys/bus/i2c/drivers/cw201x/unbind
 * artik1020: $ echo 0-0062 > /sys/bus/i2c/drivers/cw201x/unbind
 * artik710 : $ echo 8-0062 > /sys/bus/i2c/drivers/cw201x/unbind
 * artik530 : $ echo 8-0062 > /sys/bus/i2c/drivers/cw201x/unbind
 * Eagleye530: $ echo 6-001a > /sys/bus/i2c/drivers/rt5659/unbind
 */

const artik = require('../src');
const name = artik.get_platform_name();

if(name == 'ARTIK 520') {
  console.log('Running I2C test on ARTIK 520');
  test_cw2015(artik.i2c(1, 2000, '8', 0x62));
} else if(name == 'ARTIK 1020') {
  console.log('Running I2C test on ARTIK 1020');
  test_cw2015(artik.i2c(0, 2000, '8', 0x62));
} else if(name == 'ARTIK 710') {
  console.log('Running I2C test on ARTIK 710');
  test_cw2015(artik.i2c(8, 2000, '8', 0x62));
} else if(name == 'ARTIK 530') {
  console.log('Running I2C test on ARTIK 530');
  test_cw2015(artik.i2c(8, 2000, '8', 0x62));
} else if(name == 'ARTIK 305') {
  console.log('Running I2C test on ARTIK 305');
  test_cw2015(artik.i2c(8, 2000, '8', 0x62));
} else if (name == 'Eagleye530') {
  console.log('Running I2C test on Eagleye530');
  test_rtc5659(artik.i2c(6, 2000, '16', 0x1a));
} else {
  console.log('Unrecognized platform');
  process.exit(-1);
}

function test_cw2015(cw2015) {
  if (cw2015.request()) {
    console.log('Failed to request cw2015');
  } else {
    console.log('Version: ' + Buffer(cw2015.read_register(0, 1)).toString('hex'));
    var reg = cw2015.read_register(8, 1);
    console.log('Config: 0x' + Buffer(reg).toString('hex'));
    reg = new Buffer([0xff], 'hex');
    console.log('Writing 0x' + Buffer(reg).toString('hex') + ' to config register');
    cw2015.write_register(8, reg);
    reg = cw2015.read_register(8, 1);
    console.log('Config: 0x' + Buffer(reg).toString('hex'));

    cw2015.release();
  }
}

function test_rtc5659(rtc5659) {
  if (rtc5659.request()) {
    console.log('Failed to request rtc5659');
  } else {
    console.log('Device ID: ' + Buffer(rtc5659.read_register(0xff00, 2)).toString('hex'));
    var reg = rtc5659.read_register(0xfb00, 2);
    console.log('Dummy: 0x' + Buffer(reg).toString('hex'));
    reg = new Buffer([0xaa, 0x55], 'hex');
    console.log('Writing 0x' + Buffer(reg).toString('hex') + ' to dummy register');
    rtc5659.write_register(0xfb00, reg);
    reg = rtc5659.read_register(0xfb00, 2);
    console.log('Dummy: 0x' + Buffer(reg).toString('hex'));

    rtc5659.release();
  }
}
