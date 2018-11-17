var bluetooth = require('../src/bluetooth');

var tx_buff = new Buffer('aabbccddeeff');
var profile = new bluetooth.SppProfile("ARTIK SPP", "spp char loopback", "server", 22, 3, true, false, true, 10, 20);
var spp = new bluetooth.Spp();

spp.on("new_connection", function(spp_socket) {

    console.log("Device connected : ");
    console.log("version = " + spp_socket.version);
    console.log("features = " + spp_socket.features);
    spp_socket.on("data", function(error, buffer) {
	console.log("received data = " + buffer);
    });
    spp_socket.on("disconnect", function(spp_socket) {
	console.log("disconnect");
    });

    spp_socket.write(tx_buff);
});

spp.on("release", function(spp_socket) {
    console.log("release");
})

spp.register_profile(profile);
process.on('SIGINT', function() {
    spp.unregister_profile();
});
