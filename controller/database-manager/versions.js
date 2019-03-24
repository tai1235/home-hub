// Dependencies
let fs = require('fs');
let path = require('path');

let versionsPath = path.join(__dirname, 'versions');

const Versions = {
    // Update collection's version, if collection not exist, create new one
    create: (collection, version) => {
        let fd = fs.openSync(versionsPath + '/' + collection + '.json', 'w');
        if (fd) {
            fs.writeFileSync(fd, JSON.stringify({ version }));
            fs.closeSync(fd);
            return true;
        } else {
            return false;
        }
    },

    get: (collection) => {
        let data = fs.readFileSync(versionsPath + '/' + collection + '.json');
        return JSON.parse(data.toString()).version;
    },

    update: (collection) => {
        let version = Versions.get(collection);
        Versions.create(collection, ++version);
    },

    list: () => {
        let versionList = [];
        let filename = fs.readdirSync(versionsPath + '/');
        if (filename) {
            for (let file of filename) {
                let data = fs.readFileSync(versionsPath + '/' + file);
                let collection = file.replace('.json', '');
                let version = JSON.parse(data.toString('utf8')).version;
                versionList.push({ collection, version })
            }
            return versionList;
        } else {
            return false;
        }
    }
};

module.exports = Versions;