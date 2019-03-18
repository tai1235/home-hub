/**
 * Author: TaiNV
 * Date Created: 2019/02/17
 * Module: db-manager
 * Description: Rule schema definition
 */

// Dependencies
let mongoose = require('mongoose');
let Logger = require('../../libraries/system-log');

let logger = new Logger(__filename);

let GroupSchema = new mongoose.Schema({
    _id: mongoose.Schema.Types.ObjectId,
    name: {
        type: String,
        required: true,
        trim: true
    },
    endpoints: [{
        eui64: {
            type: String,
            required: true,
            trim: true
        },
        endpoint: {
            type: Number,
            required: true
        }
    }],
    enable: {
        type: Boolean,
        required: true,
        default: true
    },
});

const Groups = new mongoose.model('Groups', GroupSchema);

const GroupsDB = {
    addGroup: () => {},
    removeGroup: () => {},
    enableGroup: () => {},
    getAllGroups: callback => {
        Groups.find({})
            .exec()
            .then(groups => callback(groups))
            .catch(err => logger.error(err.message));
    },
    handleGroupAdd: () => GroupsDB.addGroup(),
    handleGroupRemove: () => GroupsDB.removeGroup(),
    handleGroupEnable: () => GroupsDB.enableGroup()
};

module.exports = GroupsDB;