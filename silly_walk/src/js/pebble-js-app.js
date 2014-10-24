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
	Pebble.openURL('data:text/html,'+encodeURI('<!DOCTYPE html><!-- -*-coding: utf-8 -*-vim: sw=2 ts=2 expandtab ai--><html>  <head>    <meta name="viewport" content="width=device-width, initial-scale=1">    <style>      body { background-color: black; text-align: center; color: white }      h1 { margin: 0 }      small { color: gray }      a { color: white }      input { height: 1.5em; font-size: 1.2em; font-weight: bold }      .text { width: 93%; margin: 0.5em; text-align: center }      .submit { width: 93%; margin: 0.4em }      .param { display: inline-table; width: 95%; height: 3em }      .label,.checkbox { display: table-cell; vertical-align: middle }      .label { text-align: left }      .checkbox { text-align: right; width: 1.5em; height: 1.5em }      .example { width: 75%; display: inline-block; text-align: left; font-size: 0.6em }    </style>    <script>      var config = _CONFIG_;      function put_config() {        for (var param in config) {          var element = document.getElementById(param);          if (element) {            if (typeof config[param] === \'boolean\') {              element.checked = config[param];            } else {              element.value = config[param];            }          }        }      }      function get_config() {        var form = document.getElementById(\'config_form\');        for (config = {}, i = 0; i < form.length ; i++) {          id = form[i].id;          if (id != "save") {            if (form[i].type === \'checkbox\') {              config[id] = form[i].checked;            } else {              config[id] = form[i].value;            }          }        }       	return window.location.href = "pebblejs://close#" + encodeURIComponent(JSON.stringify(config));      }      function toggle_visibility(id) {        var e = document.getElementById(id);        if(e.style.display == \'block\')          e.style.display = \'none\';        else          e.style.display = \'block\';      }    </script>  </head>  <body onload="put_config();">    <h1>Silly Walk</h1>    <small>by Dan Long, mod by Alexey Avdyukhin</small>    <hr size="1" />    <form action="javascript: get_config();" id="config_form">      <div class="param">        <div class="label">          Show "no connection" icon<br>          <small>Show when phone is not connected</small>        </div>        <div class="checkbox">          <input type="checkbox" id="config_show_no_phone" class="checkbox">        </div>      </div>      <div class="param">        <div class="label">          Show battery icon<br>          <small>Show battery status</small>        </div>        <div class="checkbox">          <input type="checkbox" id="config_show_battery" class="checkbox">        </div>      </div>      <div class="param">        <div class="label">          Vibe on disconnect<br>          <small>Vibe when phone is disconnected</small>        </div>        <div class="checkbox">          <input type="checkbox" id="config_vibe" class="checkbox">        </div>      </div>			<div class="param">        <div class="label">          Seconds<br>          <small>Show seconds</small>        </div>        <div class="checkbox">          <input type="checkbox" id="config_seconds" class="checkbox">        </div>      </div>			      <div class="param">        <div class="label">          Simple mode<br>          <small>Use simple background</small>        </div>        <div class="checkbox">          <input type="checkbox" id="config_simple" class="checkbox">        </div>      </div>									<hr size="1" />			<input type="submit" id="save" class="submit" value="Save and apply">  </body></html><!--.html'.replace('_CONFIG_', JSON.stringify(options), 'g')));
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
