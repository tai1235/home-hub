const Coordinator = require('./coordinator/coordinator');
const Logger = require('./libraries/system-log');

const logger = new Logger(__filename)
const coordinator = new Coordinator();

coordinator.start();
coordinator.process();