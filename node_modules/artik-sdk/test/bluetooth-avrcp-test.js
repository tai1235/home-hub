/* Global Includes */
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
var path 	   = require('path');

var avrcp;
var bt;

var remote_addr = process.env.BT_ADDR;
var is_browsable = process.env.BT_AVRCP_IS_BROWSABLE;
var has_repeat_mode_support = parseInt(process.env.BT_AVRCP_HAS_REPEAT_MODE_SUPPORT);
var has_name_support = process.env.BT_AVRCP_HAS_NAME_SUPPORT;
var has_type_support = process.env.BT_AVRCP_HAS_TYPE_SUPPORT;
var item_add_to_playing = process.env.BT_AVRCP_ITEM;
var runManualTests = parseInt(process.env.RUN_MANUAL_TESTS);

var is_skipped = !remote_addr || !remote_addr.length;

testCase('Avrcp', function() {
	pre(function(done) {
		this.timeout(10000);
		if (!runManualTests)
			this.skip();

		if (is_skipped)
			done();

		bt = new bluetooth();
		bt.on('connect', function(err, connected) {
			console.log('Device ' + remote_addr + ' is ' + connected);
			avrcp = new bluetooth.Avrcp()
			done();
		});
		bt.on('bond', function(err, paired) {
			console.log('Device ' + remote_addr + ' is ' + paired);
			bt.connect(remote_addr);
		});
		bt.start_bond(remote_addr);
	});

	testCase('#controller_change_folder', function() {
		assertions('Change folder', function() {
			if (is_skipped || !is_browsable || !runManualTests)
				this.skip();

			var items = avrcp.controller_list_item(-1 , -1);

			if (items.length == 0)
				this.skip();

			var folder = null;

			for (var i = 0, len = items.length; i < len; i++) {
				var item = items[i];

				if (item.property.type == "folder") {
					folder = item;
					break;
				}
			}

			if (folder == null) {
				this.skip();
			}

			avrcp.controller_change_folder(folder.object_path);
		});
	});

	testCase('#controller_list_item', function() {
		assertions('List item of current folder', function() {
			if (is_skipped || !is_browsable || !runManualTests)
				this.skip();

			var items = avrcp.controller_list_item(-1, -1);
			assert.isArray(items);

			if (items.length == 0)
				this.skip();

			for (var i = 0, len = items.length; i < len; i++) {
				var item = items[i];

				assert.isString(item.object_path);
				assert.isObject(item.property);
				assert.isString(item.property.player);
				assert.isString(item.property.name);
				assert.isString(item.property.type);
				assert.include(['folder', 'audio', 'video'], item.property.type);

				if (item.property.type == 'folder') {
					assert.isString(item.property.folder);
					assert.include(['mixed', 'titles', 'albums', 'artists'], item.property.folder);
					assert.isUndefined(item.property.playable);
					assert.isUndefined(item.property.title);
					assert.isUndefined(item.property.artist);
					assert.isUndefined(item.property.album);
					assert.isUndefined(item.property.genre);
					assert.isUndefined(item.property.number_of_tracks);
					assert.isUndefined(item.property.number);
					assert.isUndefined(item.property.duration);
				} else {
					assert.isUndefined(item.property.folder);

					assert.isBoolean(item.property.playable);
					if (item.property.title != null)
						assert.isString(item.property.title);
					if (item.property.artist != null)
						assert.isString(item.property.artist);
					if (item.property.album != null)
						assert.isString(item.property.album);
					if (item.property.genre != null)
						assert.isString(item.property.genre);
					assert.isNumber(item.property.number_of_tracks);
					assert.isNumber(item.property.number);
					assert.isNumber(item.property.duration);
				}
			}
		});
	});

	testCase('#controller_get_repeat_mode', function() {
		assertions('Get repeat mode', function() {
			if (is_skipped || !has_repeat_mode_support || !runManualTests)
				this.skip();

			var repeat_mode = avrcp.controller_get_repeat_mode();
			assert.isString(repeat_mode);
			assert.include(['off', 'singletrack', 'alltracks', 'group'], repeat_mode);
		});

		assertions('Set repeat mode', function() {
			if (is_skipped || !has_repeat_mode_support || !runManualTests)
				this.skip();

			avrcp.controller_set_repeat_mode('alltracks');
			assert.equal(avrcp.controller_get_repeat_mode(), 'alltracks');
		});

		assertions('Set repeat mode to invalid value', function(done) {
			if (is_skipped || !has_repeat_mode_support || !runManualTests)
				this.skip();

			assert.throws(avrcp.controller_set_repeat_mode('invalid'), 'Repeat');
		});
	});

	testCase('#controller_is_connected', function() {
		assertions('Controller is connected', function() {
			if (is_skipped || !runManualTests)
				this.skip();

			assert.isOk(avrcp.controller_is_connected());
		});

		assertions('Controller is not connected', function(done) {
			if (is_skipped || !runManualTests)
				this.skip();

			/* This test is not working well */
			this.skip();

			bt.on('connect', function(err, connected){
				assert.isNotOk(avrcp.controller_is_connected());
				done();
			});
			bt.disconnect(remote_addr);
		});
	});

	testCase('#controller_resume_play', function(){
		assertions('Resume play', function() {
			if (is_skipped || !runManualTests)
				this.skip();

			avrcp.controller_resume_play();
			this.timeout(2000);
			setTimeout(function() {
				var status = avrcp.controller_get_status();
				assert.equal(status, 'playing');
			}, 200);
		})
	});

	testCase('#controller_pause', function(){
		assertions('Pause', function() {
			if (is_skipped || !runManualTests)
				this.skip();

			this.timeout(2000);
			avrcp.controller_pause();
			setTimeout(function() {
				var status = avrcp.controller_get_status();
				assert.equal(status, 'paused');
			}, 200);
		})
	});

	testCase('#controller_stop', function(){
		assertions('Stop', function() {
			if (is_skipped || !runManualTests)
				this.skip();

			this.timeout(2000);
			avrcp.controller_stop();
			setTimeout(function() {
				var status = avrcp.controller_get_status();
				assert.equal(status, 'stopped');
			}, 200);
		});
	});

	testCase('#controller_next', function(){
		assertions('Next', function() {
			if (is_skipped || !runManualTests)
				this.skip();

			this.timeout(2000);
			avrcp.controller_next();
			setTimeout(function() {
				var status = avrcp.controller_get_status();
				assert.equal(status, 'playing');
			}, 200);
		});
	});

	testCase('#controller_previous', function(){
		assertions('Previous', function() {
			if (is_skipped || !runManualTests)
				this.skip();

			this.timeout(2000);
			avrcp.controller_previous();
			setTimeout(function() {
				var status = avrcp.controller_get_status();
				assert.equal(status, 'playing');
			}, 200);
		});
	});

	testCase('#controller_fast_forward', function(){
		assertions('Fast forward', function() {
			if (is_skipped || !runManualTests)
				this.skip();

			this.timeout(2000)
			avrcp.controller_fast_forward();
			setTimeout(function() {
				var status = avrcp.controller_get_status();
				assert.equal(status, 'forward-seek');
			}, 200);
		});
	});

	testCase('#controller_rewind', function() {
		assertions('Rewind', function() {
			if (is_skipped || !runManualTests)
				this.skip();

			this.timeout(2000);
			avrcp.controller_rewind();
			setTimeout(function() {
				var status = avrcp.controller_get_status();
				assert.equal(status, 'reverse-seek');
			}, 200);
		});
	});

	testCase('#controller_get_property', function() {
		assertions('Get property', function() {
			if (is_skipped || !is_browsable || !item_add_to_playing ||
				!item_add_to_playing.length || !runManualTests)
				this.skip();

			/* This test is not working well */
			this.skip();
			avrcp.controller_change_folder(path.dirname(item_add_to_playing))
			avrcp.controller_list_item(-1, -1);
			var property = avrcp.controller_get_property(item_add_to_playing)

			assert.isObject(property);
			assert.isString(property.player);
			assert.isString(property.name);
			assert.isString(property.type);
			assert.include(['folder', 'audio', 'video'], property.type);

			if (property.type == 'folder') {
				assert.isString(property.folder);
				assert.include(['mixed', 'titles', 'albums', 'artists', 'playlists'], property.folder);
				assert.isUndefined(property.playable);
				assert.isUndefined(property.title);
				assert.isUndefined(property.artist);
				assert.isUndefined(property.album);
				assert.isUndefined(property.genre);
				assert.isUndefined(property.number_of_tracks);
				assert.isUndefined(property.number);
				assert.isUndefined(property.duration);
			} else {
				assert.isUndefined(property.folder);

				assert.isBoolean(property.playable);
				if (property.title != null)
					assert.isString(property.title);
				if (property.artist != null)
					assert.isString(property.artist);
				if (property.album != null)
					assert.isString(property.album);
				if (property.genre != null)
					assert.isString(property.genre);
				assert.isNumber(property.number_of_tracks);
				assert.isNumber(property.number);
				assert.isNumber(property.duration);
			}
		});
	});

	testCase('#controller_add_to_playing', function() {
		assertions('Add', function() {
			if (is_skipped || !item_add_to_playing ||
				!item_add_to_playing.length || !runManualTests)
				this.skip();

			avrcp.controller_add_to_playing(item_add_to_playing);
			avrcp.controller_next();
			setTimeout(function() {
				var status = avrcp.controller_get_status();
				assert.equal(status, 'playing');
			}, 200);
		});
	});

	testCase('#controller_get_name', function() {
		assertions('Get name', function() {
			if (is_skipped || !has_name_support || !runManualTests)
				this.skip();

			assert.isString(avrcp.controller_get_name());
		});
	});

	testCase('#controller_get_status', function() {
		assertions('Get current status', function() {
			if (is_skipped || !runManualTests)
				this.skip();

			var status = avrcp.controller_get_status();
			assert.isString(avrcp.controller_get_status());
			assert.include(['playing', 'stopped', 'paused', 'forward-seek', 'reverse-seek', 'error'], status);
		});
	});

	/* not tested, due to the lack of software with subtype */
	testCase('#controller_get_subtype', function() {
		assertions('Get subtype', function(){
			if (is_skipped || !has_type_support || !runManualTests)
				this.skip();

			var subtype = avrcp.controller_get_subtype()
			assert.isString(subtype);
			assert.include(['Audio Book', 'Podcast'], subtype)
		});
	});

	/* not tested, due to the lack of software with type */
	testCase('#controller_get_type', function() {
		assertions('Get type', function() {
			if (is_skipped || !has_type_support || !runManualTests)
				this.skip();

			var type = avrcp.controller_get_type();
			assert.isString(avrcp.controller_get_type());
			assert.include(['Audio', 'Video', 'Audio Broadcasting', 'Video Broadcasting'], type);
		});
	});

	testCase('#controller_is_browsable', function() {
		assertions('Is browsable', function() {
			if (is_skipped || !is_browsable || !runManualTests)
				this.skip();

			assert.isBoolean(avrcp.controller_is_browsable());
			console.log("Browsable : " + avrcp.controller_is_browsable());
		});
	});

	testCase('#controller_get_position', function() {
		assertions('Get position', function() {
			if (is_skipped || !runManualTests)
				this.skip();

			assert.isNumber(avrcp.controller_get_position());
		});
	});

	post(function() {
	});
});
