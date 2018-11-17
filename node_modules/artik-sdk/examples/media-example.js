const artik = require('../src');

var media = artik.media();

var sound_file = '/usr/share/sounds/alsa/Front_Center.wav';

console.log('Playing: ' + sound_file);

media.play_sound_file(sound_file, function(response, status) {
	console.log('Finished playing');
	process.exit(0);	
});
