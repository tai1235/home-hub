var bluetooth = require('../src/bluetooth');
var CommandLine = require('./commandline');
var bt = new bluetooth();
var ftp = new bluetooth.Ftp();

var remote_addr = null;

ftp.on('transfer', function(properties) {
	console.log("Transfer file: " + properties.filename + " - " + properties.status + " - " + properties.transfered + "/" + properties.size);
});

function get_file(args, help) {
	if (args.length != 2) {
		help();
		return;
	}

	ftp.get_file(args[0], args[1]);
	console.log("Get file " + args[0] + " from " + remote_addr + "... in progress");
}

function put_file(args, help) {
	if (args.length != 2) {
		help();
		return;
	}

	ftp.put_file(args[0], args[1]);
	console.log("Put file " + args[0] + " to " + remote_addr + "... in progress");
}

function list_files(args, help) {
	var files = ftp.list_folder();

    files.forEach(function(e) {
	console.log("Filename: " + e.filename);
	console.log("File type: " + e.type);
	console.log("Modified: " + e.modified);
	console.log("Permission: " + e.permission);
	console.log("Size: " + e.size);
	console.log("");
    });
	console.log("List current directory succeeded");
}

function delete_file(args, help) {
	if (args.length != 1) {
		help();
		return;
	}

	ftp.delete_file(args[0]);
	console.log("Delete file " + args[0] + " succeeded");
}

function change_directory(args, help) {
	if (args.length != 1) {
		help();
		return;
	}

	ftp.change_folder(args[0]);
	console.log("Change current folder to " + args[0] + " succeeded");
}

function make_directory(args, help) {
	if (args.length != 1) {
		help();
		return;
	}

	ftp.create_folder(args[0]);
	console.log("Create new folder " + args[0] + " succeeded");
}

function cancel_transfer(args, help) {
	ftp.cancel_transfer();
	console.log("Cancel transfer succeeded");
}

function resume_transfer(args, help) {
	ftp.resume_transfer();
	console.log("Resume transfer succeeded");
}

function suspend_transfer(args, help) {
	ftp.suspend_transfer();
	console.log("Suspend transfer succeeded");
}

function quit(args) {
	process.exit(0);
}

var commands = [
	{
		command: "get",
		description: "Get file",
		doc: "get remote_source_file local_target_file",
		handler: get_file
	},
	{
		command: "put",
		description: "Put file",
		doc: "put local_source_file remote_target_file",
		handler: put_file
	},
	{
		command: "ls",
		description: "List files",
		doc: "ls",
		handler: list_files
	},
	{
		command: "delete",
		description: "Delete file",
		doc: "delete file",
		handler: delete_file
	},
	{
		command: "cd",
		description: "Change the current folder",
		doc: "cd folder",
		handler: change_directory
	},
	{
		command: "mkdir",
		description: "Create new folder",
		doc: "mkdir folder",
		handler: make_directory
	},
	{
		command: "cancel",
		description: "Cancel transfer file",
		doc: "cancel",
		handler: cancel_transfer
	},
	{
		command: "resume",
		description: "Resume transfer file",
		doc: "resume",
		handler: resume_transfer
	},
	{
		command: "suspend",
		description: "Suspend transfer file",
		doc: "suspend",
		handler: suspend_transfer
	},
	{
		command: "quit",
		description: "Quit",
		doc: "quit",
		handler: quit
	}
];

var command_line = new CommandLine(commands);

bt.on('started', function() {
	console.log('onstarted');
	bt.start_scan();
});


function stopScanning() {
	bt.stop_scan();
	console.log("Please input FTP server MAC address:");
	command_line.read_line(function (line) {
		remote_addr = line;
		bt.start_bond(line);
	});
}

setTimeout(stopScanning, 20000);

bt.on('scan', function(err, device) {
	console.log('onscan (err = ' + err + '): ' + device);
});

bt.on('bond', function(err, paired) {
	console.log('bonded (err=' + err + '): ' + paired);
	if (!err) {
		console.log("Connect to " + remote_addr);
		ftp.create_session(remote_addr);
		command_line.process();
	}
});

process.on('SIGINT', function() {
	if (bt.is_scanning())
		bt.stop_scan();

	process.exit(0);
});
