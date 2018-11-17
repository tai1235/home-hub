var exec = require('child_process').execSync;
var wifi = require('../src/wifi');

var wifi_ap = new wifi.wifi_ap();

var ssid = "SimpleTestAP"; //'<enter a SSID here>';
var pwd = "test1234"; //'<passphrase of the SSID>';
var channel = 1; //'<Number channel of the AP>';
var openmode = false; //'<En/Disable wpa2 authentification>'

if (process.argv.length >= 3)
	ssid = process.argv[2];
if (process.argv.length >= 4)
	pwd = process.argv[3];
if (process.argv.length >= 5)
	channel = Number(process.argv[4]);
if (process.argv.length >= 6)
	openmode = (process.argv[5] == "true");

wifi_ap.on('started', function() {
	console.log("Starting Wi-Fi Access Point");
	console.log("SSID : '" + ssid + "'");
	console.log("Pass : '" + pwd + "'");
	console.log("Channel : '" + channel + "'");
	console.log("OpenMode : '" + openmode + "'");
	var res = wifi_ap.start_ap(ssid, pwd, channel, openmode ?
			wifi_ap.WIFI_ENCRYPTION_OPEN : wifi_ap.WIFI_ENCRYPTION_WPA2);
	console.log("Result => '" + res + "'");
	process.exit(0);
});
