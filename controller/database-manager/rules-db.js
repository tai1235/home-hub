/**
 * Author: TaiNV
 * Date Created: 2019/02/17
 * Module: database-manager
 * Description: Rule schema definition
 */

// Dependencies
let mongoose = require('mongoose');
let EventEmitter = require('events');
let Logger = require('../../libraries/system-log');

let logger = new Logger(__filename);

let RuleSchema = new mongoose.Schema({
    _id: mongoose.Schema.Types.ObjectId,
    type: {
        type: Number,
        required: true
    },
    name: {
        type: String,
        required: true,
        trim: true
    },
    input: {
        device: [{
            endpoint: {
                eui64: {
                    type: String,
                    required: true,
                    trim: true
                },
                id: {
                    type: Number,
                    required: true
                }
            },
            value: {
                type: Object,
                required: true
            },
            condition: {
                type: Number,
                required: true
            }
        }],
        schedule: {
            startTime: {
                hour: {
                    type: Number,
                    required: true
                },
                minute: {
                    type: Number,
                    required: true,
                }
            },
            endTime: {
                hour: {
                    type: Number,
                    required: true
                },
                minute: {
                    type: Number,
                    required: true,
                }
            },
            repeat: {
                date: [{
                    type: String,
                    trim: true
                }],
            }
        }
    },
    output: {
        device: [{
            endpoint: {
                eui64: {
                    type: String,
                    required: true,
                    trim: true
                },
                id: {
                    type: Number,
                    required: true
                }
            },
            value: {
                type: Object,
                required: true
            },
            timer: {
                type: Number,
                required: true
            }
        }]
    },
    enable: {
        type: Boolean,
        required: true,
        default: true
    },
});

class Rules extends EventEmitter {
    constructor() {
        super();
        this.rules = new mongoose.model('Rules', RuleSchema)
    }

    getAllRules(callback) {
        this.rules.find({})
            .exec()
            .then(rules => callback(rules))
            .catch(err => logger.error(err.message));
    }
    handleRuleAdd() {

    }

    handleRuleRemove() {

    }

    handleRuleEnable() {

    }

    handleRuleActive() {

    }
}

module.exports = Rules;