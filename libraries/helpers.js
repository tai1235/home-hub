// Dependencies
var config = require('./config');
var crypto = require('crypto');
// var https = require('https');
// var querystring = require('querystring');

// Container for all the helpers
var helpers = {};

// Parse a JSON string to an object in all cases, without throwing
helpers.parseJsonToObject = function (str) {
    try {
        return JSON.parse(str);
    } catch (e) {
        return {};
    }
};

// Create a SHA256 hash
helpers.hash = str => {
    if (typeof (str) == 'string' && str.length > 0) {
        return crypto.createHmac('sha256', config.hashingSecret).update(str).digest('hex');
    } else {
        return false;
    }
};

// Create a string of random alphanumeric characters, of a given length
helpers.createRandomString = strLength => {
    strLength = typeof (strLength) == 'number' && strLength > 0 ? strLength : false;
    if (strLength) {
        // Define all the possible characters that could go into a string
        let possibleCharacters = 'abcdefghijklmnopqrstuvwxyz' +
                                 'ABCDEFGHIJKLMNOPQRSTUVWXYZ' +
                                 '0123456789';

        // Start the final string
        let str = '';
        for (let i = 1; i <= strLength; i++) {
            // Get a random charactert from the possibleCharacters string
            let randomCharacter = possibleCharacters.charAt(Math.floor(Math.random() * possibleCharacters.length));
            // Append this character to the string
            str += randomCharacter;
        }
        // Return the final string
        return str;
    } else {
        return false;
    }
};


// Export the module
module.exports = helpers;