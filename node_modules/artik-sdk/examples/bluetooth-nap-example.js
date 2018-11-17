var child_process = require('child_process');
var bluetooth = require('../src/bluetooth');
var bt = new bluetooth();
var pan = new bluetooth.Pan();

function check_interface_exists(itf) {
    var lines = require('fs').readFileSync('/proc/net/dev').toString().split('\n');

    // Drop header lines
    lines = lines.slice(2);

    lines.forEach(function (line) {
	if (line.includes(itf)) {
	    return true;
	}
    });
}

var args = process.argv.slice(2);

if (args.length < 5) {
    console.log("Input parameter error");
    console.log("Please input as node " + process.argv[1] + " bridge ip netmask dhcp_begin dhcp_end");
    process.exit(1);
}

var bridge = args[0];
var nap_ip = args[1];
var nap_netmask = args[2];
var nap_dhcp_begin = args[3];
var nap_dhcp_end = args[4];

var system = null;

if (check_interface_exists(bridge)) {
    system = child_process.spawnSync("brctl", ["delbr", bridge]);
    if (system.status != 0) {
	console.log("failed to remove " + bridge);
	process.exit(1);
    }
}

system = child_process.spawnSync("brctl", ["addbr", bridge]);
if (system.status != 0) {
    console.log("failed to add " + bridge);
    process.exit(1);
}

system = child_process.spawnSync("ip", ["addr", "add", nap_ip, "dev", bridge]);
if (system.status != 0) {
    console.log("config " + bridge + " address failed");
    process.exit(1);
}

system = child_process.spawnSync("ip", [ "link", "set", bridge, "up"]);
if (system.status != 0) {
    console.log("up " + bridge + " failed");
    process.exit(1);
}

system = child_process.spawnSync("sysctl", ["-w", "net.ipv4.ip_forward=1"])
if (system.status != 0) {
    console.log("enable ip forward failed");
    process.exit(1);
}

system = child_process.spawnSync("ifconfig", [bridge, "netmask", nap_netmask, "up"]);
if (system.status != 0) {
    console.log("config " + bridge + " netmask failed");
    process.exit(1);
}

system = child_process.spawnSync("iptables", ["-t", "nat", "-A", "POSTROUTING", "-s", nap_ip + "/" +nap_netmask, "-j", "MASQUERADE"]);
if (system.status != 0) {
    console.log("configure iptables failed");
    process.exit(1);
}
system = child_process.spawnSync("iptables", ["-t", "filter", "-A", "FORWARD", "-i", bridge, "-j", "ACCEPT"]);
if (system.status != 0) {
    console.log("configure iptables failed");
    process.exit(1);
}

system = child_process.spawnSync("iptables", ["-t", "filter", "-A", "FORWARD", "-o", bridge, "-j", "ACCEPT"]);
if (system.status != 0) {
    console.log("configure iptables failed");
    process.exit(1);
}

var dhcp_range = nap_dhcp_begin + "," + nap_dhcp_end + ",60m";
system = child_process.spawnSync("/usr/sbin/dnsmasq", ["--pid-file=/var/run/dnsmaq." + bridge + ".pid", "--bind-interfaces", "--dhcp-range=" + dhcp_range, "--except-interface=lo", "--interface="+bridge, "--dhcp-option=option:router," + nap_ip]);
if (system.status != 0) {
    console.log("Launch dnsmasq failed");
    process.exit(1);
}

pan.register("nap", bridge);

process.on('SIGINT', function() {
	pan.unregister("nap");

	child_process.spawnSync("ifconfig", [bridge, "down"]);
	child_process.spawnSync("brctl", ["delbr", bridge]);
	child_process.spawnSync("pkill", ["-9", "dnsmasq"]);
	child_process.spawnSync("iptables", ["-t", "nat", "-D", "POSTROUTING", "-s", nap_ip+"/"+nap_netmask, "-j", "MASQUERADE"]);
    process.exit(1);
});
