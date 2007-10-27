// sms_timer.js
//
// Called periodically to handle sms messages

debug('SMS Timer called');
debug('Started is ', $started);
if ("0" == $started) {
	sms_init();
} else {
	sms_read_ack();
}
var obj = new Object();
obj.started = true;
var n = (1 * now());
run('','startup/sms_timer.js', n + 60,obj);
debug('SMS Timer done');

