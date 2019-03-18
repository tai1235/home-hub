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

let today = new Date();
let date = String(today.getDate()).padStart(2, '0');
let month = String(today.getMonth() + 1).padStart(2, '0'); //January is 0!
let year = today.getFullYear();

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
                type: {
                    type: Number,
                    required: true
                },
                data: {
                    date: {
                        type: String,
                        default: '1111111',
                        trim: true
                    },
                    aboutTime: {
                        after: {
                            type: Number,
                            default: 1
                        },
                        day: {
                            type: Number,
                            default: date
                        },
                        month: {
                            type: Number,
                            default: month
                        },
                        year: {
                            type: Number,
                            default: year
                        }
                    }
                }
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

const Rules = new mongoose.model('Rules', RuleSchema);

const RulesDB = {
    addRule: () => {},
    removeRule: () => {},
    enableRule: () => {},
    activeRule: () => {},
    getAllRules: callback => {
        Rules.find({})
            .exec()
            .then(rules => callback(rules))
            .catch(err => logger.error(err.message));
    },
    handleRuleAdd: () => RulesDB.addRule(),
    handleRuleRemove: () => RulesDB.removeRule(),
    handleRuleEnable: () => RulesDB.enableRule(),
    handleRuleActive: () => RulesDB.activeRule()
};

module.exports = RulesDB;