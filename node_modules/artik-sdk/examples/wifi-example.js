var wifi = require('../src/wifi');

var ssid = '<enter a SSID here>';
var pwd = '<passphrase of the SSID>';

var wifi_station = new wifi.wifi_station();

wifi_station.on('started', function() {
	console.log("Current Wifi Information: " + wifi_station.get_info());
	wifi_station.scan_request();
});

wifi_station.on('connected', function() {
	console.log('connected');
	process.exit(0);
});

wifi_station.on('scan', function(list) {
	var results = JSON.parse(list);
	console.log(results);
	var ap = results.filter(function(item) {
		return item.name == ssid;
	});

	if (ap.length > 0) {
		console.log('Found SSID ' + ssid + ', connecting...');
		wifi_station.disconnect();
		var ret = wifi_station.connect(ssid, pwd, false);
		if (ret != 'OK') {
			console.log('Failed to connect to SSID ' + ssid + ' - ' + ret);
			process.exit(-1);
		}
	}
});

process.on('SIGINT', function () {
	process.exit(0);
});
