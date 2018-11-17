const artik = require('../src');

var json = JSON.parse(artik.get_device_info());
console.log('Device name: ' + json.name);
console.log('Modules: ');
for (i in json.modules)
	console.log('\t' + json.modules[i]);


const platform = artik.get_platform_name();
const bt_mac_addr = artik.get_bt_mac_address();
const wifi_mac_addr = artik.get_wifi_mac_address();
const serial_number = artik.get_platform_serial_number();
const manufacturer = artik.get_platform_manufacturer();
const uptime = artik.get_platform_uptime();
const model_number = artik.get_platform_model_number();

console.log('Device name: ' + platform);
console.log('Bluetooth mac address: ' + bt_mac_addr);
console.log('Wifi mac address: ' + wifi_mac_addr);
console.log('Platform serial number: ' + serial_number);
console.log('Platform manufacturer: ' + manufacturer);
console.log('Platform uptime: ' + uptime);
console.log('Platform model number: ' + model_number);

