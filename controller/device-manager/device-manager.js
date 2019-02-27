/**
 * Author: TaiNV
 * Date Created: 2019/02/17
 * Module: device-manager
 * Description: provide management for all device connect to this hub
 */

// Dependencies


// Container for all method
const device_manager = {};

// Array contains all devices
device_manager.devices = [];

device_manager.cluster_id = [
    "0x0000",
    "0x0006"
]

device_manager.HandleDeviceJoined = (data) => {
    // Create devices to store
    delete data.timeSinceLastMessage;
    device.devices.push(data);

    // Store to database

    // Create HAP accessory

    // Announce to server

}

// Export the module
module.exports = device;