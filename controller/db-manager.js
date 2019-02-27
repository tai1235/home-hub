/**
 * Author: TaiNV
 * Date Created: 2019/02/17
 * Module: db-manager
 * Description: provide database interface
 */

// Dependencies
const mongo = require('mongodb').MongoClient;
const ObjectId = require('mongodb').ObjectID;

const _url = "mongodb://localhost:27017";
const _dbName = "home-hub";
const _options = { useNewUrlParser: true };

// Container of the module
const db_manager = {};

// All database used in this project
db_manager.database = {
    db: null
}

db_manager.collection = {
    devices: null,
    rules: null
}

// Connect function
db_manager.connect = (callback) => {
    if (db_manager.database.db) {
        callback(false);
    } else {
        mongo.connect(_url, _options, (err, client) => {
            if (err) {
                callback(err);
            } else {
                db_manager.database.db = client.db(_dbName);
                db_manager.collection.devices = client.db(_dbName).collection("devices");
                db_manager.collection.rules = client.db(_dbName).collection("rules");
                callback(false);
            }
        });
    }
}

db_manager.getPrimaryKey = id => {
    return ObjectId(id);
}

db_manager.getDB = () => {
    return db_manager.database.db
}

db_manager.getCollection = collection => {
    return db_manager.collection[collection];
}

// Export the module
module.exports = db_manager;