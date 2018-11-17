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

var os		   = require('os');
var ifaces 	   = os.networkInterfaces();

var net_config = {
	ip_addr: process.env.NETWORK_IP_ADDRESS,

	netmask: process.env.NETWORK_NETMASK,

	gw_addr: process.env.NETWORK_GW_ADDRESS,

	dns_addr: []
}

/* Test Case Module */
testCase('Network', function() {

	pre(function() {
		network = new artik.network(true);
	});

	testCase('#get_network_config()', function() {
		postEach('Enabling Wifi', function(done){
			this.timeout(15000);
			exec("ifconfig wlan0 up; sleep 1; pkill dhclient; sleep 1; dhclient wlan0;sleep 1");
			done();
		});

		assertions('Get Network Config - Should return config not null', function(done) {
			this.timeout(2000);
			var config = network.get_network_config(process.env.NETWORK_INTERFACE);
			if (config) {
				console.log("IP Address: " + config.ip_addr);
				console.log("Netmask: " + config.netmask);
				console.log("Gateway: " + config.gw_addr);
				console.log("DNS Address 1: " + config.dns_addr[0]);
				console.log("DNS Address 2: " + config.dns_addr[1]);
				console.log("MAC Address: " + config.mac_addr);
			}
			assert.isNotNull(config);
			assert.equal(validator.isIP(config.ip_addr), true);
			assert.equal(validator.isIP(config.netmask), true);
			assert.equal(validator.isIP(config.gw_addr), true);
			assert.equal(validator.isIP(config.dns_addr[0]), true);
			assert.equal(validator.isIP(config.dns_addr[1]), true);
			assert.equal(validator.isMACAddress(config.mac_addr), true);
			done();
		});

	});

	testCase('#get_online_status()', function() {
		postEach('Enabling Wifi', function(done){
			this.timeout(15000);
			exec("ifconfig wlan0 up; sleep 1; pkill dhclient; sleep 1; dhclient wlan0;sleep 1");
			done();
		});


		assertions('Online Status - Should return true when it is online', function(done) {
			this.timeout(2000);
			var online_status = network.get_online_status();
			console.log('Status ' + online_status);
			assert.equal(online_status, true);
			done();
		});

		assertions('Online Status - Should return false when it is offline', function(done) {
			this.timeout(10000);
			console.log("Disabling WIFI Adapter");
			exec("ifconfig wlan0 down; sleep 1");
			console.log("Check Online Status")
			var online_status = network.get_online_status();
			assert.equal(online_status, false);
			done();
		});

	});

	testCase('#get_current_public_ip()', function() {

		postEach('Enabling Wifi', function(done) {
			this.timeout(15000);
			exec("ifconfig wlan0 up; sleep 1; pkill dhclient; sleep 1; dhclient wlan0;sleep 1");
			done();
		});

		assertions('Get Current Public Ip - Should return a valid IP', function(done) {
				this.timeout(2000);
				var ip = network.get_current_public_ip();
				console.log("IP Address: " + ip);
				assert( validator.isIP(ip), true );
				done();
		});

		assertions('Get Current Public Ip - Should return null when there is no connectivity', function(done) {
				exec("ifconfig wlan0 down");
				try {
					var ip = network.get_current_public_ip();
					console.log("IP Address: " + ip);
					assert( ip, null );
				} catch (e) {
					console.log("Exception caught while getting ip");
					assert(false);
				}
				done();
		});
	});

	testCase("#on connectivity-change", function() {
		pre(function() {
			this.timeout(5000);
			console.log("Enabling wifi");
			exec("ifconfig wlan0 up; pkill dhclient; dhclient wlan0");
		});

		postEach('Enabling Wifi', function(done){
			this.timeout(10000);
			network.removeAllListeners("connectivity-change");
			done();
		});

		assertions('Event network-status-change - Should raise this event when deconnection occurs', function(done) {
			this.timeout(10000);
			network.on("connectivity-change", function(status) {
				assert.equal(status, false);
				done();
			});
			exec("ifconfig wlan0 down;");
		});

		assertions('Event network-status-change - Should raise this event when reconnection occurs', function(done) {
			this.timeout(10000);
			exec("ifconfig wlan0 up; pkill dhclient; dhclient wlan0");
			network.on("connectivity-change", function(status){
				assert.equal(status, true);
				done();
			});
		});
	});

	testCase("#dhcp_client", function(){
		pre(function() {
			console.log("Setting no ip address");
			exec("pkill dhclient; ifconfig wlan0 0.0.0.0");
		});

		assertions("Get IP Address from a DHCP server - Should return an IP address when there is response", function(done) {
			var ip;
			network.dhcp_client_start("wifi");

			Object.keys(ifaces).forEach(function (ifname) {
				var alias = 0;

				ifaces[ifname].forEach(function (iface) {
					if ('IPv4' !== iface.family || iface.internal !== false || ifname !== "wlan0") {
						// skip over internal (i.e. 127.0.0.1) and non-ipv4 addresses
						return;
					}

					if (alias >= 1) {
						// this single interface has multiple ipv4 addresses
						console.log(ifname + ':' + alias, iface.address);
					} else {
						// this interface has only one ipv4 adress
						ip = iface.address;
					}
					++alias;
				});
			});

			console.log("IP Address: " + ip);
			assert.equal(validator.isIP(ip), true);
			network.dhcp_client_stop();
			done();
		})

	});

	testCase('#set_network_config()', function() {
		postEach('Enabling Wifi', function(done){
			this.timeout(15000);
			exec("ifconfig wlan0 up; sleep 1; pkill dhclient; sleep 1;");
			done();
		});

		assertions('Set Network Config - Should return expected config', function(done) {
			this.timeout(5000);

			if (process.env.NETWORK_DNS_ADDRESS_1)
				net_config.dns_addr.push(process.env.NETWORK_DNS_ADDRESS_1);

			if (process.env.NETWORK_DNS_ADDRESS_2)
				net_config.dns_addr.push(process.env.NETWORK_DNS_ADDRESS_2);

			if (!net_config.ip_addr || !net_config.netmask || !net_config.gw_addr ||
				!net_config.dns_addr[0])
				this.skip();

			var ret = network.set_network_config(net_config, process.env.NETWORK_INTERFACE);

			assert.equal(ret, 0);

			var new_config = network.get_network_config(process.env.NETWORK_INTERFACE);
			if (new_config) {
				console.log("IP Address: " + new_config.ip_addr);
				console.log("Netmask: " + new_config.netmask);
				console.log("Gateway: " + new_config.gw_addr);
				console.log("DNS Address 1: " + new_config.dns_addr[0]);
				console.log("DNS Address 2: " + new_config.dns_addr[1]);
				console.log("MAC Address:" + new_config.mac_addr);
			}

			if (!net_config.dns_addr[1])
				net_config.dns_addr[1] = "0.0.0.0";

			assert.isNotNull(new_config);
			assert.equal(new_config.ip_addr, net_config.ip_addr);
			assert.equal(new_config.netmask, net_config.netmask);
			assert.equal(new_config.gw_addr, net_config.gw_addr);
			assert.equal(new_config.dns_addr[0], net_config.dns_addr[0]);
			assert.equal(new_config.dns_addr[1], net_config.dns_addr[1]);
			done();
		});

	});
});
