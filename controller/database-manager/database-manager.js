/**
 * Author: TaiNV
 * Date Created: 2019/02/17
 * Module: db-manager
 * Description: provide database interface
 */

// Dependencies
let mongoose = require('mongoose');
let DevicesDB = require('./devices-db');
let RulesDB = require('./rules-db');
let GroupsDB = require('./groups-db');
const Logger = require('../../libraries/system-log');

let logger = new Logger(__filename);

const DatabaseManager = {
    start: (callback) => {
        logger.info('START mongodb client');
        mongoose.connect('mongodb://localhost:27017/home-hub', {
            useNewUrlParser: true,
        }).then(() => {
            logger.debug('Connected to database server');
            callback();
        }).catch(err => {
            logger.error(err.message);
        });
    },

    getAllDevices: callback => DevicesDB.getAllDevices(callback),
    getAllRules: callback => RulesDB.getAllRules(callback),
    getAllGroups: callback => GroupsDB.getAllGroups(callback),

    // Device handlers
    handleDeviceJoined: (eui64, endpoint, type) => DevicesDB.handleDeviceJoined(eui64, endpoint, type),
    handleDeviceLeft: eui64 => DevicesDB.handleDeviceLeft(eui64),
    handleDeviceUpdate: (eui64, params) => DevicesDB.handleDeviceUpdate(eui64, params),
    handleDeviceRemove: eui64 => DevicesDB.handleDeviceRemove(eui64),

    // Rule handlers
    handleRuleAdd: () => RulesDB.handleRuleAdd(),
    handleRuleRemove: () => RulesDB.handleRuleRemove(),
    handleRuleEnable: () => RulesDB.handleRuleEnable(),
    handleRuleActive: () => RulesDB.handleRuleActive(),

    // Group handlers
    handleGroupAdd: () => GroupsDB.handleGroupAdd(),
    handleGroupRemove: () => GroupsDB.handleGroupRemove(),
    handleGroupEnable: () => GroupsDB.handleGroupEnable()
};

module.exports = DatabaseManager;