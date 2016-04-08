var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

function fetchParty(id) {
  var url = 'http://festiwatch.pebble.sillevis.net/party/' + id;
  xhrRequest(url, 'GET',
    function(responseText) {
      // responseText contains a JSON object with party info
      var response = JSON.parse(responseText);

      console.log('response ' + responseText);

      var dictionary = {
        'MUSIC_ICON_KEY': 0,
        'MUSIC_DJ_KEY': 'name',
        'MUSIC_TIME_KEY': 'time',
        'MUSIC_AREA_KEY': 'area',
        'PARTY_ID_KEY': id
      };

      Pebble.sendAppMessage(dictionary, function(e) {
        console.log('Party info sent to Pebble successfully!');
        console.log(JSON.stringify(e));
      }, function() {
        console.log('Error sending party info to Pebble!');
      });
    });
}

function fetchParties(latitude, longitude) {
  var params;
  if(latitude && longitude) {
    params = '?latitude=' + latitude + '&longitude=' + longitude + '&radius=50';
  } else {
    params = '?radius=50';
  }
  var url = 'http://festiwatch.pebble.sillevis.net/' + params;
   // Send request to Festiwatch Backend
  xhrRequest(url, 'GET', 
    function(responseText) {
      // responseText contains a JSON object with party info
      var response = JSON.parse(responseText);

      var name = '';
      var location = '';
      var visitors = '0';
      var id = 0;
      if('success' in response && response.success === true && response.data.length > 0) {
        name = response.data[0].name;
        location = response.data[0].location;
        visitors = response.data[0].visitors;
        id = response.data[0].id;
      }

      console.log('name ' + name);
      console.log('location ' + location);
      console.log('visitors ' + visitors);
      console.log('id ' + id);

      var dictionary = {
        'PARTY_NAME_KEY': name,
        'PARTY_LOCATION_KEY': location,
        'PARTY_VISITOR_KEY': visitors.toString(),
        'PARTY_ID_KEY': id
      };

      Pebble.sendAppMessage(dictionary, function() {
        console.log('Party info sent to Pebble successfully!');
      }, function() {
        console.log('Error sending party info to Pebble!');
      });
    }      
  );
}

function locationSuccess(pos) {
  if(pos && 'coords' in pos && typeof pos.coords === 'object' && 'latitude' in pos.coords && 'longitude' in pos.coords && pos.coords.latitude && pos.coords.longitude) {
    fetchParties(pos.coords.latitude, pos.coords.longitude);
  }
  else {
    fetchParties();
  }
}

function locationError(err) {
  console.warn('location error (' + err.code + '): ' + err.message);
  var dictionary = {
    'PARTY_NAME_KEY': 'Loc Unavailable',
    'PARTY_LOCATION_KEY': 'N/A',
    'PARTY_VISITOR_KEY': 0,
    'PARTY_ID_KEY': 0
  };

  Pebble.sendAppMessage(dictionary);
}

var locationOptions = {
  'timeout': 15000,
  'maximumAge': 60000
};

Pebble.addEventListener('ready', function (e) {
  console.log('connect!');
  window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError,
    locationOptions);
  console.log(e.type);
});

Pebble.addEventListener('appmessage', function (e) {
  console.log('message!');
  console.log(JSON.stringify(e));
  if(e.payload && 'PARTY_ID_KEY' in e.payload) {
    console.log('getting party ' + e.payload.PARTY_ID_KEY);
    fetchParty(e.payload.PARTY_ID_KEY);
  }
  else {
    console.log('getting all parties');
    window.navigator.geolocation.getCurrentPosition(locationSuccess, locationError,
      locationOptions);
  }
});

Pebble.addEventListener('webviewclosed', function (e) {
  console.log('webview closed');
  console.log(e.type);
  console.log(e.response);
});
