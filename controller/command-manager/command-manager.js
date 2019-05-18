let DevicesCommand = require('./commands/devices-command');
let RulesCommand = require('./commands/rules-command');
let GroupsCommand = require('./commands/groups-command');
let Commands = require('./command');

module.exports = {
    Commands,
    CommandData: {
        Devices: DevicesCommand,
        Rules: RulesCommand,
        Groups: GroupsCommand
    }
};