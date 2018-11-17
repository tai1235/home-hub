var bluetooth = require('../src/bluetooth');
var readline = require('readline');

function Request(display_cb, process_cb) {
	this.display_cb = display_cb;
	this.process_cb = process_cb;
}

var requests = [];
var agent = new bluetooth.Agent();
var bt = new bluetooth();

agent.on("request_pincode", function(request, device) {
	console.log("request pincode");
	var req = new Request(
		function() { console.log("Enter the pincode of " + device); },
		function(pincode) { request.send_pincode(pincode); }
	);

	requests.push(req);
});

agent.on("display_pincode", function(device, pincode) {
	console.log("display_pincode");
	var req = new Request(
		function() { console.log("The pincode for the device " + device + " is " + pincode); },
		null
	);

	requests.push(req);
});

agent.on("request_passkey", function(request, device) {
	console.log("request_passkey");

	var req = new Request(
		function() { console.log("Enter the passkey of " + device); },
		function(passkey) { request.send_passkey(passkey); });

	requests.push(req);
});

agent.on("display_passkey", function(device, passkey, entered) {
	console.log("display_passkey");
	var req = new Request(
		function() { console.log("The passkey for the device " + device + " is " + passkey); },
		null );

	requests.push(req);
});

agent.on("confirmation", function(request, device, passkey) {
	console.log("confirmation");
	var req = new Request(
		function() { console.log("Confirm the passkey " + passkey + " for the device? (yes/no)" + device); },
		function(confirmation) { if (confirmation == "yes") request.send_empty_response(); });

	requests.push(req);
});

agent.on("authorization", function(request, device) {
	console.log("authorization");
	var req = new Request(
		function() { console.log("Request authorization for the device? (yes/no)" + device); },
		function(confirmation) { if (confirmation == "yes") request.send_empty_response(); }
	);

	requests.push(req);
});

agent.on("authorize_service", function(request, device, uuid) {
	console.log("authorize_service");
	var req = new Request(
		function() { console.log("Authorize service " + uuid + " for device " + device + "? (yes/no)"); },
		function(confirmation) { if (confirmation == "yes") request.send_empty_response(); });

	requests.push(req);
});

var current_request = null;
var r1 = readline.createInterface({
	input: process.stdin,
	output: process.stdout,
	terminal: false
});

r1.on("line", function(line) {
	if (current_request == null)
		return;

	current_request.process_cb(line);
	current_request = null;
});


setInterval(function() {
	if (requests.length == 0)
		return;

	if (current_request != null)
		return;

	var request = requests.pop();
	request.display_cb();

	if (request.process_cb != null)
		current_request = request;
}, 100);

bt.set_discoverable(true);
agent.register_capability("keyboard_only");
agent.set_default();
