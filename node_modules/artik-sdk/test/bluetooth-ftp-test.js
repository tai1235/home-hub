var testCase   = require('mocha').describe;
var pre        = require('mocha').before;
var preEach    = require('mocha').beforeEach;
var post       = require('mocha').after;
var postEach   = require('mocha').afterEach;
var assertions = require('mocha').it;
var assert     = require('chai').assert;
var validator  = require('validator');
var exec       = require('child_process').execSync;
var bluetooth  = require('../src/bluetooth');
const fs 	   = require('fs');

var remote_addr = process.env.BT_ADDR;
var remote_file = process.env.BT_REMOTE_FILE;
var src_file = process.env.BT_SOURCE_FILE;
var runManualTests = parseInt(process.env.RUN_MANUAL_TESTS);

var is_skipped = !remote_addr || !remote_addr.length;

var ftp = new bluetooth.Ftp();

testCase('Folder and file operations', function(done) {
	pre(function() {
		this.timeout(10000);
		if (is_skipped || !runManualTests)
			this.skip();

		ftp.create_session(remote_addr);
	});

	assertions('Folder operations', function() {
		if (is_skipped || !runManualTests)
			this.skip();

		var files = ftp.list_folder();

		for (var i = 0; i < files.length; i++) {
			assert.isObject(files[i]);
			assert.isString(files[i].type);
			assert.isString(files[i].filename);
			assert.isString(files[i].modified);
			assert.isString(files[i].permission);
			assert.isNumber(files[i].size);
		}

		ftp.create_folder("myfolder");
		ftp.change_folder("../");

		files = ftp.list_folder();
		var names = files.map(function(x) { return x.filename })
		assert.include(names, "myfolder");
		ftp.delete_file("myfolder");

		files = ftp.list_folder();
		var names = files.map(function(x) { return x.filename })
		assert.notInclude(names, "myfolder");
	});

	assertions('Get a file', function(done) {
		this.timeout(60000);
		if (is_skipped || !runManualTests)
			this.skip();

		if (!remote_file || !remote_file.length)
			this.skip();

		fs.unlink("/tmp/test", (err) => {});

		ftp.on('transfer', function(properties) {
			console.log("status = " + properties.status);
			assert.notEqual("error", properties.status);
			if (properties.status == "complete") {
				done();
				ftp.removeAllListeners();
			}
		});

		ftp.get_file("/tmp/test", remote_file);
	});

	assertions('Put a file', function(done) {
		this.timeout(20000);
		if (is_skipped || !runManualTests)
			this.skip();

		if (!src_file || !src_file.length)
			this.skip();

		ftp.on('transfer', function(properties) {
			if (properties.status == "complete") {
				ftp.delete_file("artik_test");
				done();
			}
		});

		ftp.put_file(src_file, "artik_test");
	});

	post(function() {
		if (is_skipped || !runManualTests)
			this.skip();

		ftp.remove_session();
	});
});
