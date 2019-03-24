/**
 * Author: TaiNV
 * Date Created: 2019/02/17
 * Module: db-manager
 * Description: provide database interface
 */

// Dependencies
let mongoose = require('mongoose');
let EventEmitter = require('events');
let Devices = require('./devices-db');
let Rules = require('./rules-db');
let Groups = require('./groups-db');
const Logger = require('../../libraries/system-log');

let logger = new Logger(__filename);

class DatabaseManager extends EventEmitter {
    constructor() {
        super();
        this.devices = new Devices();
        this.rules = new Rules();
        this.groups = new Groups();
    }

    start(callback) {
        logger.info('START mongodb client');
        mongoose.connect('mongodb://localhost:27017/home-hub', {
            useNewUrlParser: true,
        }).then(() => {
            logger.debug('Connected to database server');
            callback();
        }).catch(err => {
            logger.error(err.message);
            setTimeout(() => {
                this.start(callback);
            }, 5000);
        });
    }

    getAllDevices(callback) {
        this.devices.getAllDevices(callback)
    }
    getAllRules(callback) {
        this.rules.getAllRules(callback)
    }
    getAllGroups(callback) {
        this.groups.getAllGroups(callback)
    }

    // Device handlers
    handleDeviceJoined(eui64, endpoint, type) {
        this.devices.handleDeviceJoined(eui64, endpoint, type, (e, params) => {
            if (!e) this.emit('database-device-added', params);
        });
    }
    handleDeviceLeft(eui64) {
        this.devices.handleDeviceLeft(eui64, (e, params) => {
            if (!e) this.emit('database-device-offline', params);
        })
    }
    // handleDeviceUpdate(eui64, params) {
    //     this.devices.handleDeviceUpdate(eui64, params)
    // }
    handleDeviceRemove(eui64) {
        this.devices.handleDeviceRemove(eui64, (e, params) => {
            if (!e) this.emit('database-device-removed', params);
        })
    }
    handleDeviceStatus(eui64) {
        this.devices.handleDeviceStatus(eui64, (e, params) => {
            if (!e) this.emit('database-device-online', params);
        })
    }

    // Rule handlers
    handleRuleAdd() {
        this.rules.handleRuleAdd();
    }
    handleRuleRemove() {
        this.rules.handleRuleRemove()
    }
    handleRuleEnable() {
        this.rules.handleRuleEnable()
    }
    handleRuleActive() {
        this.rules.handleRuleActive()
    }

    // Group handlers
    handleGroupAdd() {
        this.groups.handleGroupAdd()
    }
    handleGroupRemove() {
        this.groups.handleGroupRemove()
    }
    handleGroupEnable() {
        this.groups.handleGroupEnable()
    }
}

module.exports = DatabaseManager;