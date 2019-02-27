/**
 * Author: TaiNV
 * Date Created: 2019/02/16
 * Module: mqtt-client
 * Description: provide interprocess communication by using mqtt
 */

// Dependencies
var storage = require('node-persist');
var uuid = require('./').uuid;
var Bridge = require('./').Bridge;
var Accessory = require('./').Accessory;
var logging = require('../libraries/system-log').create(__filename);

// Container for all methods
var hap = {};

hap.bridge = new Bridge("Homehub", uuid.generate("Homehub"));

// Create new HAP accessory
hap.newAccessory = accessoryData => {
    // Validate accessory's data
    var type = typeof (accessoryData.type) == 'string' && ['light', 'switch'].indexOf(accessoryData.type) ? accessoryData.type : false;
    var numberOfService = typeof (accessoryData.numberOfService) == 'number' ? accessoryData.numberOfService : false;
    var name = typeof (accessoryData.name) == 'string' && accessoryData.name.length > 0 ? accessoryData.name : false;
    var serialNumber = typeof (accessoryData.serialNumber) == 'string' ? accessoryData.serialNumber : false;
    var manufacturer = typeof (accessoryData.manufacturer) == 'string' ? accessoryData.manufacturer : false;
    var model = typeof (accessoryData.model) == 'string' ? accessoryData.manufacturer : false;

    if (type && numberOfService && name && serialNumber && manufacturer && model) {
        if (type == 'switch') {
            // Generate uuid for switch accessory
            var switchUUID = uuid.generate('hap-nodejs:accessories:switch' + name);
            // The accessory created by this function
            var switchAccessory = new Accessory(name, switchUUID);
            // Add services
            switchAccessory
                .getService(Service.AccessoryInformation)
                .setCharacteristic(Characteristic.Manufacturer, manufacturer)
                .setCharacteristic(Characteristic.Model, model)
                .setCharacteristic(Characteristic.SerialNumber, serialNumber);
            switchAccessory.on('identify', (paired, callback) => {
                logging.info("Identified " + name + " :" + paired);
                // Invoke the callback
                callback();
            });
            for (var i = 0; i < numberOfService; i++) {
                switchAccessory
                    .addService(Service.Switch, name + '.' + i)
                    .getCharacteristic(Characteristic.On)
                    .on('set', (value, callback) => {
                        // Send control command to device on Zigbee network

                        // Invoke the callback
                        callback();
                    })
                    .on('get', callback => {
                        // Callback the status of accessory
                        // callback(null, getStatus());
                    })
                    .updateValue(true);
            }

            hap.bridge.addBridgedAccessory(switchAccessory);
            return switchAccessory;
        } else if (type == 'light') {
            // Generate uuid for switch accessory
            var lightUUID = uuid.generate('hap-nodejs:accessories:light' + name);
            // The accessory created by this function
            var lightAccessory = new Accessory(name, lightUUID);
            // Add services
            lightAccessory
                .getService(Service.AccessoryInformation)
                .setCharacteristic(Characteristic.Manufacturer, manufacturer)
                .setCharacteristic(Characteristic.Model, model)
                .setCharacteristic(Characteristic.SerialNumber, serialNumber);
            lightAccessory.on('identify', (paired, callback) => {
                logging.info("Identified " + name + " :" + paired);

                // Invoke the callback
                callback();
            });
            for (var i = 0; i < numberOfService; i++) {
                lightAccessory
                    .addService(Service.Lightbulb, name + '.' + i)
                    .getCharacteristic(Characteristic.On)
                    .on('set', (value, callback) => {
                        // Send state control command to device on Zigbee network

                        // Invoke the callback
                        callback();
                    })
                    .on('get', callback => {
                        // Callback the state of accessory
                        // callback(null, getStatus());
                    })
                    .updateValue(true);
                lightAccessory
                    .getService(name + '.' + i)
                    .addCharacteristic(Characteristic.Brightness)
                    .on('set', (value, callback) => {
                        // Send level control command to device on Zigbee network

                        // Invoke the callback
                        callback();
                    })
                    .on('get', callback => {
                        // Callback the level of accessory
                        // callback(null, getStatus());
                    });
            }
            hap.bridge.addBridgedAccessory(lightAccessory);
            return lightAccessory;
        }
    }
}

hap.init = () => {
    logging.info("HAP starting...");
    storage.initSync();

    bridge.on('identify', function (paired, callback) {
        logging.info("Node Bridge identify");
        callback(); // success
    });

    bridge.publish({
        username: "CC:22:3D:E3:CE:F6",
        port: 51826,
        pincode: "031-45-154",
        category: Accessory.Categories.BRIDGE
    });
}

// Export the module
module.exports = hap;