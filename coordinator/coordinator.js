/**
 * Author: TaiNV
 * Date Created: 2019/02/26
 * Module: device
 * Description: device implementation
 */

// Dependencies
const ZigbeeGateway = require('./zigbee-gateway').ZigbeeGateway;
const EventEmitter = require('events');
const Logger = require('../libraries/system-log');
const DeviceManager = require('../controller/device-manager/device-manager');
const Device = require('../controller/device-manager/device');
const config = require('../config');
const storage = require('node-persist');

const logger = new Logger(__filename);

class Coordinator {
    constructor() {
        this.zigbeeGateway = new ZigbeeGateway(config.gatewayId);
        this.zigbeeGateway.getConnectStatus(() => {
            this.zigbeeGateway.process()
        });
        this.deviceManager = new DeviceManager()
    }

    handleDeviceJoined() {
        this.zigbeeGateway.on('device-joined', params => {
            // Create and store new device to cache
            this.deviceManager.handleDeviceJoined(params.eui64, params.endpoint, params.type);

            // Store device's data to DB

            // Invoke the callback
            // callback();
        })
    }

    handleDeviceLeft(callback) {
        this.zigbeeGateway.on('device-left', message => {
            // Parse parameter from message

            // Create and store new device to cache
            this.deviceManager.handleDeviceLeft(eui64);

            // Store device's data to DB

            // Invoke the callback
            callback();
        })
    }

    handleDeviceStatus() {
        this.zigbeeGateway.on('device-response', params => {
            // Handle device status
            console.log(params);
            this.deviceManager.handleDeviceStatus(params.value, params.eui64, params.endpoint);

            // Handle rule input

            // Handle group input

            // Invoke the callback
            // callback()
        })
    }

    handleRuleAdd(callback) {

    }

    handleRuleRemove(callback) {

    }

    handleRuleActive(callback) {

    }

    handleRuleEnable(callback) {

    }

    handleGroupAdd(callback) {

    }

    handleGroupRemove(callback) {

    }

    handleGroupEnable(callback) {

    }

    start() {
        // Initiate the device manager
        storage.initSync();
        this.deviceManager.start();
    }

    process() {
        // Handle device's event
        this.handleDeviceJoined();
        this.handleDeviceLeft();
        this.handleDeviceStatus();
        // Handle rule's event
        this.handleRuleAdd();
        this.handleRuleRemove();
        this.handleRuleEnable();
        this.handleRuleActive();
        // Handle group's event
        this.handleGroupAdd();
        this.handleGroupRemove();
        this.handleGroupEnable();
    }
}

module.exports = Coordinator