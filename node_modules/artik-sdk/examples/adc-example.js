const artik = require('../src');

var adc = artik.adc(0, "adcjs-test");

var intervalFunc = setInterval(function() {
    console.log("launch request by interval");
    adc.request();
    console.log("[%s] : load", adc.get_name());
    console.log("[%s] :  get_value %d .", adc.get_name(), adc.get_value());
    adc.release();
    console.log("Release ADC module.");
}, 800);

process.on('SIGINT', function () {
    console.log("Kill adc  ");
    clearInterval(intervalFunc);
    process.kill(process.pid);
});
