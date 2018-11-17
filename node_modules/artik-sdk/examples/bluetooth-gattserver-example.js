var GattServer = require("../src/bluetooth").GattServer;

/*
+ * Make the GATT server object global to prevent it
+ * from being garbage collected when running in the
+ * background.
+ */
global. gatt_server = new GattServer();
var val = 100;

var refId = null;
var countDown = function(updateValueCallback)
{
	val--;
	if (val < 0) {
		clearInterval(refId);
		process.exit(1);
	}
	updateValueCallback(new Buffer([val]));
}

global.gatt_server.start_advertising({
	type: "peripheral",
	serviceUuids: [ "00001802-0000-1000-8000-00805f9b34fb", "0000180f-0000-1000-8000-00805f9b34fb" ],
});

var descriptor = new GattServer.Descriptor({
	uuid: "ffffffff-ffff-ffff-ffff-abcdabcdabcd",
	properties: ["read", "write"],
	value: new Buffer([0x12]),
	onWriteRequest: function(buf, callback) {
		callback("ok");
	}
});

var characteristic = new GattServer.Characteristic({
	uuid: "00002a19-0000-1000-8000-00805f9b34fb",
	properties: ["read", "notify", "write"],
	value: new Buffer([0x66]),
	descriptors: [descriptor],
	onSubscribe: function(updateValueCallback) {
		refId = setInterval(countDown, 1000, updateValueCallback);
	},
	onWriteRequest: function(buf, callback) {
		callback("ok");
	}
});

global.gatt_server.add_service({
	uuid: "0000180f-0000-1000-8000-00805f9b34fb",
	characteristics: [
		characteristic
	]
});
