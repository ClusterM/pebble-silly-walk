var initialized = false;
var options = {
  "config_show_no_phone": true,
  "config_show_battery": true,
  "config_vibe": false,
  "config_simple": false,
	"config_seconds": false
};

Pebble.addEventListener("ready", function() {
  initialized = true;
	var json = window.localStorage.getItem('silly-walk-config');	
	if (typeof json === 'string') {
		try {
      options = JSON.parse(json);
			Pebble.sendAppMessage(options);
			console.log("Loaded stored config: " + json);
    } catch(e) {
      console.log("stored config json parse error: " + json + ' - ' + e);
    }
  }
});

Pebble.addEventListener("showConfiguration", function() {
  console.log("showing configuration");
	Pebble.openURL('data:text/html,'+encodeURI('_HTMLMARKER_<!--.html'.replace('_CONFIG_', JSON.stringify(options), 'g')));
});

Pebble.addEventListener("webviewclosed", function(e) {
	var response = decodeURIComponent(e.response);
  if (response.charAt(0) == "{" && response.slice(-1) == "}" && response.length > 5) {
		window.localStorage.setItem('silly-walk-config', response);
		try {
			options = JSON.parse(response);
			Pebble.sendAppMessage(options);
		} catch(e) {
			console.log("Response config json parse error: " + response + ' - ' + e);
		}
    console.log("Options = " + response);
  }
});
