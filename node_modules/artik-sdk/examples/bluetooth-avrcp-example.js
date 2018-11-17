var bluetooth = require('../src/bluetooth');
var CommandLine = require('./commandline');
var bt = new bluetooth();
var avrcp = new bluetooth.Avrcp();

var remote_addr = null;

function list_item(args, help) {
	var is_browsable = avrcp.controller_is_browsable();
	if (!is_browsable) {
		console.log("list-item is not supported by " + remote_addr);
		return;
	}

	var begin = -1;
	var end = -1;
	if (args.length == 2) {
		begin = parseInt(args[0], 10);
		end = parseInt(args[1], 10)
	}

	var items = avrcp.controller_list_item(begin, end);
	items.forEach(function(e) {
		console.log("Index: #" + e.index);
		console.log("Displayable name: " + e.property.name);
		console.log("Playable: " + e.property.playable);
		console.log("Type: " + e.property.type);
		if (e.property.type == "folder") {
			console.log("Folder: " + e.property.folder);
		} else {
			console.log("Title: " + e.property.title);
			console.log("Artist: " + e.property.artist);
			console.log("Album: " + e.property.album);
			console.log("Genre: " + e.property.genre);
			console.log("Number of tracks: " + e.property.number_of_tracks);
			console.log("track: " + e.property.number);
			console.log("duration: " + e.property.duration);
		}
	});
}

function change_folder(args, help) {
	var is_browsable = avrcp.controller_is_browsable();
	if (!is_browsable)
	{
		console.log("change-folder is not supported by " + remote_addr);
		return;
	}

	if (args.length != 1)
	{
		help();
		return;
	}

	avrcp.controller_change_folder(parseInt(args[0], 10));
	console.log("Change folder to " + args[0] + "... Success");
}

function get_repeat(args, help) {
	var repeat_mode = avrcp.controller_get_repeat_mode();
	console.log("Repeat mode is " + repeat_mode);
}

function set_repeat(args, help) {
	if (args.length != 1) {
	help()
	return;
	}

	avrcp.controller_set_repeat_mode(args[0]);
	console.log("Change repeat mode to " + args[0] + "... Sucess");
}

function play_item(args, help) {
	var is_browsable = avrcp.controller_is_browsable();
	if (!is_browsable)
	{
		console.log("play-item is not supported by " + remote_addr);
		return;
	}

	if (args.length != 1) {
		help();
		return;
	}

	console.log("Play " + args[0]);
	avrcp.controller_play_item(parseInt(args[0], 10));
}

function addtoplay(args, help) {
	var is_browsable = avrcp.controller_is_browsable();
	if (!is_browsable)
	{
		console.log("addtoplay is not supported by " + remote_addr);
		return;
	}

	if (args.length != 1) {
		help();
		return;
	}

	console.log("Add " + args[0] + " to the playing list.");
	avrcp.controller_add_to_playing(parseInt(args[0], 10));
}

function resume_play(args) {
	avrcp.controller_resume_play();
}

function next(args) {
	avrcp.controller_next();
}

function previous(args) {
	avrcp.controller_previous();
}

function pause(args) {
	avrcp.controller_pause();
}

function stop(args) {
	avrcp.controller_stop();
}

function rewind(args) {
	avrcp.controller_rewind();
}

function fast_forward(args) {
	avrcp.controller_fast_forward();
}

function metadata(args) {
	var metadata = avrcp.controller_get_metadata();
	console.log("Title: " + metadata.title);
	console.log("Artist: " + metadata.artist);
	console.log("Album: " + metadata.album);
	console.log("Genre: " + metadata.genre);
	console.log("Number of tracks: " + metadata.number_of_tracks);
	console.log("track: " + metadata.number);
	console.log("duration: " + metadata.duration);
}

function quit(args) {
	process.exit(0);
}

var commands = [
	{ command: "list-item",
	  description:"List item indexs of current folder",
	  doc: "list-item [start_index end_index]\n    list-item or list-item 1 2",
	  handler: list_item },
	{ command: "change-folder",
	  description: "Change to the specified index of folder",
	  doc: "change-folder directory\n    change-folder 1",
	  handler: change_folder },
	{ command: "get-repeat",
	  description: "Get the repeat mode.",
	  doc: "get-repeat",
	  handler: get_repeat },
	{ command: "set-repeat",
	  description: "Set the repeat mode.",
	  doc: "set-repeat [singletrack|alltracks|group|off]\n   set-repeat singletrack",
	  handler: set_repeat },
	{ command: "play-item",
	  description: "Play the specified index of item.",
	  doc: "play-item item\n    play-item 1",
	  handler: play_item },
	{ command: "addtoplay",
	  description: "Add the  specified index of item to the playing list.",
	  doc: "addtoplay item\n    addtoplay 1",
	  handler: addtoplay },
	{ command: "resume-play",
	  description: "Resume play",
	  doc: "resume-play",
	  handler: resume_play },
	{ command: "next",
	  description: "Play the next item.",
	  doc: "next",
	  handler: next },
	{ command: "previous",
	  description: "Play the previous item.",
	  doc: "previous",
	  handler: previous },
	{ command: "pause",
	  description: "Pause the playing item.",
	  doc: "pause",
	  handler: pause },
	{ command: "stop",
	  description: "Stop the playing item.",
	  doc: "stop",
	  handler: stop },
	{ command: "rewind",
	  description: "Rewind the playing item.",
	  doc: "rewind",
	  handler: rewind },
	{ command: "fast-forward",
	  description: "Fast-forward the playing item.",
	  doc: "fast-forward",
	  handler: fast_forward },
	{ command: "get-metadata",
	  description: "Get the current metadata.",
	  doc: "get-metadata",
	  handler: metadata },
	{ command: "quit",
	  description: "Quit application",
	  doc: "quit",
	  handler: quit }
];

var command_line = new CommandLine(commands);

function findDevice(item) {
	return item.address == remote_addr;
}

bt.on('started', function() {
	console.log('onstarted');
	bt.start_scan();
});

function stopScanning() {
	bt.stop_scan();
	console.log("Please input AVRCP controller MAC address:");
	command_line.read_line(function(line) {
		remote_addr = line;
		bt.start_bond(line);
	})
}

setTimeout(stopScanning, 20000);

bt.on('scan', function(err, device) {
	console.log('onscan (err = ' + err + '): ' + device);
});

bt.on('bond', function(err, paired) {
	console.log('bonded (err=' + err + '): ' + paired);
	if (!err) {
		console.log("Connect to " + remote_addr);
		bt.connect(remote_addr);
	}
});

bt.on('connect', function(err, connected) {
	console.log('connected (err=' + err + '):' + connected);
	command_line.process();
});

process.on('SIGINT', function() {
	if (bt.is_scanning())
		bt.stop_scan();

	process.exit(0);
});
