var bt = new (require('../src/bluetooth'))();

var remote_addr = "<BT device address";

function findDevice(item) {
	return item.address == remote_addr;
}

bt.on('started', function() {
	console.log('onstarted');
	bt.start_scan();
});

bt.on('scan', function(err, device) {
	console.log('onscan (err=' + err + '): ' + device);
	if (!err) {
		var dev = JSON.parse(device).find(findDevice);

		if (dev) {
			console.log('Bonding to ' + remote_addr);
			bt.stop_scan();
			bt.start_bond(dev.address);
		}
	}
});

bt.on('bond', function(err, paired) {
	console.log('bonded (err=' + err +'): ' + paired);
	if (!err)
		bt.connect(remote_addr);
});

bt.on('connect', function(err, connected) {
	console.log('connected (err=' + err + '): ' + connected);
});

process.on('SIGINT', function () {
	bt.stop_scan();
    process.exit(0);
});
