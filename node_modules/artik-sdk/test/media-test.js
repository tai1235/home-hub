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
var artik      = require('../src');

/* Test Specific Includes */
var media = artik.media();
var sound_file = process.env.MEDIA_FILE;
var start, end;

/* Test Case Module */
testCase('Media', function() {

	pre(function() {
	});

	testCase('#play_sound_file', function() {
		this.timeout(10000);
		start = new Date();

		assertions('Play the sound file', function(done) {

			if (!sound_file || !sound_file.length)
				this.skip();

			media.play_sound_file(sound_file, function(response, status) {
				end = new Date();
				var timeOfPlay = (end.getTime() - start.getTime())/1000;
				console.log('Finished playing. Seconds ' + timeOfPlay);
				assert.isAtLeast(timeOfPlay, 1);
				done();
			});
		});

	});


	post(function() {
	});

});
