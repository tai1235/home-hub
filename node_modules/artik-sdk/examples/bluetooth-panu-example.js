var child_process = require('child_process');
var readline = require('readline');
var bluetooth = require('../src/bluetooth');
var bt = new bluetooth();
var pan = new bluetooth.Pan();

bt.on('started', function() {
	console.log('onstarted');
	bt.start_scan();
});

var remote_addr = null;
var r1 = null;
function stopScanning() {
	bt.stop_scan();
	console.log("Please input NAP MAC address:");
	r1 = readline.createInterface({
		input: process.stdin,
		output: process.stdout,
		terminal: false
	});
	r1.on('line', function(line) {
		r1.close();
		remote_addr = line;
		bt.start_bond(line);
	})
}

child_process.spawnSync("systemctl", ["stop", "connman"]);
setTimeout(stopScanning, 20000);

bt.on('scan', function(err, device) {
	console.log('onscan (err = ' + err + '): ' + device);
});

bt.on('bond', function(err, paired) {
	console.log('bonded (err=' + err + '): ' + paired);
	if (!err) {
		console.log("Connect to " + remote_addr);
		pan.connect(remote_addr, "nap");
		var itf = pan.get_interface();

		var system = null;
		system = child_process.spawnSync("dhclient", [ "-r", itf]);
		if (system.status != 0)
		{
			console.log("dhclient -r " + itf + "failed");
			process.exit(1);
		}

		system = child_process.spawnSync("dhclient", [itf]);
		if (system.status != 0)
		{
			console.log("dhclient failed");
			process.exit(1);
		}

		system = child_process.spawnSync("ifconfig", [ "eth0", "down"]);
		if (system.status != 0)
		{
			console.log("ifconfig failed");
			process.exit(1);
		}

		console.log("Please enter the address IP to ping");
		r1 = readline.createInterface({
			input: process.stdin,
			output: process.stdout,
			terminal: false
		});

		r1.on('line', function(line) {
			var cli = 'ping -c 10 ' + line;
			child_process.exec(cli,{encoding:'utf8'},function (err,stdout,stderr){
			    if (err){
			        console.log(err);
			        return;
			    }
			    console.log(stdout);
			})
		});
	}
});

process.on('SIGINT', function() {
	if (bt.is_scanning())
		bt.stop_scan();

	if (pan.is_connected())
		pan.disconnect();

	r1.close();
	process.exit(0);
});
