const artik_network = require('../src/network');
var opt = require('getopt');

network = new artik_network();

var dhcp_server_config = {
	iface: "wifi",

	ip_addr: "",

	netmask: "",

	gw_addr: "",

	dns_addr: [],

	start_addr: "",

	num_leases: 0
}

try{
	opt.setopt("i:n:g:a:b:s:l:e");
} catch (e){
   switch (e.type) {
	case "unknown":
		console.log("Unknown option: -%s", e.opt);
		console.log("Usage: node network-dhcp-server-example.js [-i IP address of server] " +
			"[-n netmask] [-g gateway address] " +
			"[-a DNS address 1] [-b DNS address 2] " +
			"[-s start IP address] [-l number of leases] " +
			"[-e for ethernet] (wifi by default)");
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
	case 'i':
		dhcp_server_config.ip_addr = String(p);
		break;
	case 'n':
		dhcp_server_config.netmask = String(p);
		break;
	case 'g':
		dhcp_server_config.gw_addr = String(p);
		break;
	case 'a':
		dhcp_server_config.dns_addr.push(String(p));
		break;
	case 'b':
		dhcp_server_config.dns_addr.push(String(p));
		break;
	case 's':
		dhcp_server_config.start_addr = String(p);
		break;
	case 'l':
		dhcp_server_config.num_leases = p;
		break;
	case 'e':
		dhcp_server_config.iface = "ethernet";
		break;
	default:
		console.log("Usage: network-dhcp-server-example.js [-i IP address of server] " +
			"[-n netmask] [-g gateway address] " +
			"[-a DNS address 1] [-b DNS address 2] " +
			"[-s start IP address] [-l number of leases] " +
			"[-e for ethernet] (wifi by default)");
		process.exit(0);
    }
});

console.log("Starting DHCP Server");

if (network.dhcp_server_start(dhcp_server_config) < 0){
	console.log("Failed to open and/or request IP address...");
	process.exit(-1);
}

process.on('SIGINT', function () {
	console.log("Stopping DHCP Server")
	if (network.dhcp_server_stop() < 0){
		console.log("Failed to stop DHCP Server...");
		process.exit(-1);
	}
	else{
		console.log("DHCP Server stopped");
    	process.exit(0);
	}
});
