/**
 * Author: TaiNV
 * Date Created: 2019/02/17
 * Module: database-manager
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

class Groups {
    constructor() {
        this.groups = new mongoose.model('Groups', GroupSchema)
    }

    getAllGroups(callback) {
        this.groups.find({})
            .exec()
            .then(groups => callback(false, groups))
            .catch(e => {
                logger.error(e.message);
                callback(e);
            });
    }
    handleGroupAdd() {

    }
    handleGroupRemove() {

    }
    handleGroupEnable() {

    }
}

module.exports = Groups;