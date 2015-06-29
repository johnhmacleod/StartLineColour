/*
Pebble.getTimelineToken(
  function (token) {
    console.log('My timeline token is ' + token);
  },
  function (error) { 
    console.log('Error getting timeline token: ' + error);
  }
);
*/
var bold = localStorage.getItem('bold') ? localStorage.getItem('bold') : ''; 
var racebox = localStorage.getItem('racebox') ? localStorage.getItem('racebox') : ''; 
var vibedisconnect = localStorage.getItem('vibedisconnect') ? localStorage.getItem('vibedisconnect') : ''; 
var colourAWA = localStorage.getItem('colourAWA') ? localStorage.getItem('colourAWA') : ''; 
console.log ("Start");
console.log ('bold=' + bold);
console.log ('racebox=' + racebox);
console.log ('vibedisconnect=' + vibedisconnect);
console.log ('colourAWA='+ colourAWA);

Pebble.addEventListener('showConfiguration', function(e) {
  // Show config page
//  Pebble.openURL('http://www.patrickcatanzariti.com/find_me_anything/configurable.html?searchingFor=John');
  console.log(bold);
  Pebble.openURL('http://johnpebble.ucoz.com/startline.html?bold=' + encodeURIComponent(bold) +
                 '&racebox=' + encodeURIComponent(racebox) +
                 '&vibedisconnect=' + encodeURIComponent(vibedisconnect) +
                 '&colourawa=' + encodeURIComponent(colourAWA)
                );
});


// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    console.log("PebbleKit JS ready!");
  }
);

Pebble.addEventListener('webviewclosed',
  function(e) {
    console.log('Configuration window returned: ' + e.response);
    var options = JSON.parse(decodeURIComponent(e.response));
    var tmp = decodeURIComponent(options.bold);
    if (tmp != 'undefined') {
      bold = tmp;
      localStorage.setItem('bold', bold);
    }
    tmp = decodeURIComponent(options.racebox);
    if (tmp != 'undefined') {
      racebox = tmp;
      localStorage.setItem('racebox', racebox);
    } 
    tmp = decodeURIComponent(options.vibedisconnect);
    if (tmp != 'undefined') {
      vibedisconnect = tmp;
      localStorage.setItem('vibedisconnect', vibedisconnect);
    }
    tmp = decodeURIComponent(options.colourawa);
    if (tmp != 'undefined') {
      colourAWA = tmp;
      localStorage.setItem('colourAWA', colourAWA);
    }

      var dictionary = {
        "KEY_CONFIG_BOLD": bold == 'true' ? 1 : 0,
        "KEY_CONFIG_RACEBOX": racebox == 'true' ? 1 : 0,
        "KEY_CONFIG_VIBEDISCONNECT": vibedisconnect == 'true' ? 1 : 0,
        "KEY_CONFIG_COLOURAWA": colourAWA == 'true' ? 1 : 0,
};

      // Send to Pebble
      Pebble.sendAppMessage(dictionary,
        function(e) {
          console.log("Config info sent to Pebble successfully!");
        },
        function(e) {
          console.log("Error sending config info to Pebble!");
        }
      );
    console.log("After Sent App Msg");
    console.log ('bold='+bold);
    console.log ('racebox='+racebox);
    console.log ('vibedisconnect='+vibedisconnect);
    console.log ('colourawa='+colourAWA);
  }  
);
