/* Global Includes */
var testCase 		= require('mocha').describe;
var pre 		= require('mocha').before;
var preEach 		= require('mocha').beforeEach;
var post 		= require('mocha').after;
var postEach 		= require('mocha').afterEach;
var assertions		= require('mocha').it;
var assert 		= require('chai').assert;
var validator 		= require('validator');
var exec 		= require('child_process').execSync;
var artik_network 	= require('../src/network');
var yesno 		= require('yesno');
var runManualTests 	= process.env.RUN_MANUAL_TESTS;

network = new artik_network();

/* Test Case Module */

var dhcp_server_config = {
	iface: process.env.DHCP_SERVER_INTERFACE,

	ip_addr: process.env.DHCP_SERVER_IP_ADDRESS,

	netmask: process.env.DHCP_SERVER_NETMASK,

	gw_addr: process.env.DHCP_SERVER_GW_ADDRESS,

	dns_addr:
		[
			process.env.DHCP_SERVER_DNS_ADDRESS_1,
			process.env.DHCP_SERVER_DNS_ADDRESS_2
		],

	start_addr: process.env.DHCP_SERVER_START_ADDRESS,

	num_leases: process.env.DHCP_SERVER_NUMBER_LEASES
}


testCase('DHCP Server', function() {

	pre(function(done) {

		if (runManualTests == 1){

			this.timeout(30000);

			console.log("WARNING ! Launching the DHCP server can change IP configuration\n" +
			"of devices connected on the current network. You must for example\n" +
			"create an access point and launch after the DHCP server.\n");

			yesno.ask('Continue?', true, function(ok) {
			    if(ok) {
			        done();
			    } else {
			        throw new Error("Cancelled by user");
			    }
			});
		}
		else
			done();
	});

	assertions('Start and Stop DHCP Server', function(done){

		if (runManualTests == 0)
                	this.skip();

		var ret_start, ret_stop;
		ret_start = network.dhcp_server_start(dhcp_server_config);

		assert.equal(ret_start, 0);
		this.timeout(0);

		setTimeout(function() {
			ret_stop = network.dhcp_server_stop();
			assert.equal(ret_stop, 0);
			done();
		}, 60000);
	});
});