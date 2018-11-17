var events = require('events');
var util = require('util');
var network = require('../build/Release/artik-sdk.node').network;

var Network = function(enable_watch_online_status) {
    events.EventEmitter.call(this);
    var _ = this;

    if (enable_watch_online_status == undefined)
        enable_watch_online_status = false;

    this.network = new network(function(val) {
        _.emit('connectivity-change', val);
    }, enable_watch_online_status);
}

util.inherits(Network, events.EventEmitter);
module.exports = Network;

Network.prototype.set_network_config = function set_network_config(net_config, interface) {
    return this.network.set_network_config(net_config, interface);
}

Network.prototype.get_network_config = function get_network_config(interface) {
    return this.network.get_network_config(interface);
}

Network.prototype.get_current_public_ip = function get_current_public_ip() {
    return this.network.get_current_public_ip();
}

Network.prototype.dhcp_client_start = function dhcp_client_start(interface) {
    return this.network.dhcp_client_start(interface);
}

Network.prototype.dhcp_client_stop = function dhcp_client_stop() {
    return this.network.dhcp_client_stop();
}

Network.prototype.dhcp_server_start = function dhcp_server_start(dhcp_server_config) {
    return this.network.dhcp_server_start(dhcp_server_config);
}

Network.prototype.dhcp_server_stop = function dhcp_server_stop() {
    return this.network.dhcp_server_stop();
}

Network.prototype.get_online_status = function get_online_status() {
    return this.network.get_online_status();
}
