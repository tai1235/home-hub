/**
 * Author: TaiNV
 * Date Modified: 2019/01/19
 * Module: config
 * Description: configuration for the project
 */

// Container for all environments
let environments = require('../config');

// Determine which environment was passed as a command-line argument
let currentEnvironment = typeof (process.env.NODE_ENV) == 'string' ? process.env.NODE_ENV.toLowerCase() : '';

// Check that the current environment is one of the environments above, if not default to staging
let environmentToExport = typeof (environments[currentEnvironment]) == 'object' ?
    environments[currentEnvironment] : environments.staging;

// Export the module
module.exports = environmentToExport;