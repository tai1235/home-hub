var artik_network = require('../src/network');
var opt = require('getopt');
var network_type = 'wifi';

network = new artik_network();

try{
	opt.setopt("e");
} catch (e){
   switch (e.type) {
	case "unknown":
		console.log("Unknown option: -%s", e.opt);
		console.log("Usage: node network-dhcp-client-example.js [-e for ethernet] (wifi by default)");
		break;
	case "required":
		console.log("Required parameter for option: -%s", e.opt);
		break;
	default:
		console.dir(e);
    }
	process.exit(0);
}

opt.getopt(function (o, p){
	switch(o){
	case 'e':
		network_type = "ethernet";
		break;
	default:
		console.log("Usage: node network-dhcp-client-example.js [-e for ethernet] (wifi by default)");
		process.exit(0);
    }
});

console.log("Starting DHCP Client");

if (network.dhcp_client_start(network_type) < 0){
	console.log("Failed to open and/or request IP address...");
	process.exit(-1);
}
else
	console.log("DHCP Client started");

process.on('SIGINT', function () {
	console.log("Stopping DHCP Client")
	if (network.dhcp_client_stop() < 0){
		console.log("Failed to stop DHCP Client...");
		process.exit(-1);
	}
	else{
		console.log("DHCP Client stopped");
		process.exit(0);
	}
});