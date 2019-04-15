// Dependencies
const os = require('os');
const Coordinator = require('./coordinator/coordinator');

// Get device mac address
let ifaces = os.networkInterfaces();
let networkInformation = {};

Object.keys(ifaces).forEach(ifname => {
    ifaces[ifname].forEach(iface => {
        if (iface.family === 'IPv4' && iface.internal === false) {
            networkInformation = iface;
        }
    })
});

const coordinator = new Coordinator(networkInformation.mac);

coordinator.start();
coordinator.process();