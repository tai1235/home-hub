// Dependencies
const config = require('./config');
const Coordinator = require('./coordinator/coordinator');

const coordinator = new Coordinator();

coordinator.start();
coordinator.process();