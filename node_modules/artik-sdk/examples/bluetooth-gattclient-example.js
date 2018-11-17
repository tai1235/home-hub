var bluetooth = require("../src/bluetooth");

var SERVICE_IMMEDIATE_ALERT = "0000180f-0000-1000-8000-00805f9b34fb";
var CHAR_BATTERY_LEVEL = "00002a19-0000-1000-8000-00805f9b34fb";
var DESC_TEST = "ffffffff-ffff-ffff-ffff-abcdabcdabcd";

var filter = new bluetooth.Filter(-90, [ SERVICE_IMMEDIATE_ALERT ], "le");
var char_battery_level = null;
var bt = new bluetooth();
var remote_addr = null;

function includes(array, elem) {
	for (var i = 0; i < array.length; i++) {
		if (array[i] == elem)
			return true;
	}

	return false;
}

bt.on('started', function() {
	console.log('onstarted');
	bt.set_scan_filter(filter);
	bt.on('scan', function(err, device) {
		if (err) {
			console.log("Error: " +err);
			process.exit();
		}
		device = JSON.parse(device);
		remote_addr = device[0].address;
		bt.connect(device[0].address);
		bt.stop_scan();
	});

	bt.on('connect', function(err, connected) {
		if (err) {
			console.log("Error: " +err);
			process.exit();
		}
	});

	var gatt_client = new bluetooth.GattClient();
	gatt_client.on('servicesDiscover', function() {
		var services = gatt_client.discover_services(remote_addr);

		var immediate_alert = services.find(function(service) {
			console.log("service.uuid = " + service.uuid);
			return service.uuid == SERVICE_IMMEDIATE_ALERT
		});

		console.log("Service " + immediate_alert.uuid + " found");
		var characteristics = immediate_alert.discover_characteristics();
		char_battery_level = characteristics.find(function(chr) { return chr.uuid == CHAR_BATTERY_LEVEL});
		var descriptors = char_battery_level.discover_descriptors();
		var desc_test = descriptors.find(function(desc) { return desc.uuid == DESC_TEST});
		var read_val = desc_test.read();
		console.log("read " + read_val.length + " bytes from descriptor " + desc_test.uuid + ":");
		console.log("  - " + read_val.toString("hex"));
		var desc_val = new Buffer([0x15]);
		console.log("write " + desc_val.length + " bytes to descriptor " + desc_test.uuid + ":");
		console.log("  - " + desc_val.toString("hex"));
		desc_test.write(desc_val);
		read_val = desc_test.read();
		console.log("read" + read_val.length + " bytes from descriptor " + desc_test.uuid + ":");
		console.log("  - " + read_val.toString("hex"));
		var include_read = includes(char_battery_level.properties, "read");
		var include_write = includes(char_battery_level.properties, "write");
		if (include_read) {
			var val = char_battery_level.read();
			console.log("read  " + val.length + " bytes from characteristic " + char_battery_level.uuid + ":");
			console.log("  - " + val.toString("hex"));
		}
		if (include_write) {
			var val = new Buffer([0x10]);
			console.log("write " + val.length + " bytes to characteristic " + char_battery_level.uuid + ":");
			console.log("  - " + val.toString("hex"));
			char_battery_level.write(val);
		}
		if (include_read) {
			var val = char_battery_level.read();
			console.log("read  " + val.length + " bytes from characteristic " + char_battery_level.uuid + ":");
			console.log("  - " + val.toString("hex"));
		}

		var include_notify = includes(char_battery_level.properties, "notify");
		if (include_notify)
		{
			char_battery_level.on("data", function(buffer) {
				console.log("received " + buffer.length + " bytes from characteristic " + char_battery_level.uuid + ":");
				console.log("  - " + buffer.toString("hex"))
				if (buffer[0] == 0)
					process.exit(0);
			});
			char_battery_level.subscribe();
			process.on('SIGINT', function() {
				char_battery_level.unsubscribe();
				process.exit(0);
			});
		}
	});

	bt.start_scan();
});
