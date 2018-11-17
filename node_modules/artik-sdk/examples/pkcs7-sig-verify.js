var artik = require('../src');
var opt = require('getopt');

var security = new artik.security();

/* Get path */
var path_signature_pem = "";
var path_root_ca = "";
var path_signed_data = "";
var se_id = "";
var date = null;

Date.prototype.isValid = function () {
	/* An invalid date object returns NaN for getTime() and NaN is the only */
	/* object not strictly equal to itself. */
	return 	this.getTime() === this.getTime();
};

usage = function () {
	process.stdout.write("Usage: pkcs7-sig-verify <signature> <root CA> <signed data> [signing date] ");
	process.stdout.write("-d [signing date] -u [artik/manufacturer]\n\n");
	process.stdout.write("-s: signature - PKCS7 signature in PEM format\n");
	process.stdout.write("-r: root CA - X509 root CA certificate in PEM format\n");
	process.stdout.write("-b: signed data - file containing the signed data\n");
	process.stdout.write("-d: signing date (optional) - current signing date for rollback detection\n");
	process.stdout.write("\tJS date format YYYY-MM-DDTHH:mm:SS.XXXZ");
	process.stdout.write("\tIf not provided, rollback detection is not performed\n");
	process.stdout.write("-u: use secure element artik/manufacturer\n");
	process.stdout.write("\nA JSON formatted string with verification result and error information is output on stdout\n");
	process.stdout.write("Return value contains an error code among the following ones\n");
	process.stdout.write("\t0: success\n");
	process.stdout.write("\t-1: invalid parameters\n");
	process.stdout.write("\t-2: invalid X509 certificate\n");
	process.stdout.write("\t-3: invalid PKCS7 signature\n");
	process.stdout.write("\t-4: CA verification failed\n");
	process.stdout.write("\t-5: computed digest mismatch\n");
	process.stdout.write("\t-6: signature verification failed\n");
	process.stdout.write("\t-7: signing time rollback detected\n");
	process.exit();
};

if(process.argv.length <= 7) {
	usage();
}

try {
	opt.setopt("s:r:u:b:d::");
} catch (e) {
	switch (e.type) {
		case "unknown":
			console.log("Unknown option: -%s", e.opt);
			break;
		case "required":
			console.log("Required parameter for option: -%s",  e.opt);
			break;
		default:
			console.dir(e);
			break;
	}
	process.exit(0);
}

opt.getopt(function (o, p) {
	switch(o){
	case 's':
		path_signature_pem = String(p);
		break;
	case 'r':
		path_root_ca = String(p);
		break;
	case 'd':
		date = new Date(p);
		if (date.isValid() == false) {
			console.log("Invalid signing time");
			process.exit(-1);
		}
		break;
	case 'b':
		path_signed_data = String(p);
		break;
	case 'u':
		se_id = String(p);
		if (se_id != "artik" && se_id != "manufacturer") {
			console.log("Invalid secure element id");
			process.exit(-1);
		}
		break;
	default:
		usage();
		process.exit();
	}
});

security.pkcs7_sig_verify(path_signature_pem, path_root_ca, path_signed_data, se_id, date, function(jsonReturn) {
	console.log(JSON.stringify(jsonReturn));
});
