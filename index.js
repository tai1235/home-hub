const Logger = require('./libraries/system-log')
const logger = new Logger(__filename)

console.log(__filename)

logger.debug('fatal condition');
logger.info('critical condition');
logger.error('error condition');
logger.warn('warning condition');