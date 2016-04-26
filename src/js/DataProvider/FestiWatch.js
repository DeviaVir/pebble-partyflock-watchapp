'use strict';

var ajax = require('../lib/ajax');
var UI = require('../ui');
var current_watch = require('../WatchInfo.js');

function showLineup(lineups, name) {
  var items = [];
  lineups.forEach(function(item) {
    items.push({
      title: item.artist.name,
      subtitle: ('type' in item ? item.type.toUpperCase() + ': ' : '') + ('time_start' in item ? (item.time_start === '0000' ? 'CONTINUOUS' : item.time_start + ' - ') : '') + ('time_end' in item && item.time_end !== '0000' ? item.time_end : '')
    });
  });
  // Construct Menu to show to user
  var lineupMenu = new UI.Menu({
    sections: [{
      title: name,
      items: items
    }]
  });

  // Show the Menu, hide the loader
  lineupMenu.show();

  lineupMenu.on('select', function(e) {
    // show something?
  });
}

var provider = {
  loadSingle: function(loading, error, id) {
    loading.show();
    
    var url = 'http://festiwatch.pebble.sillevis.net/party/' + id;
    ajax({
      url: url,
      method: 'GET',
      type: 'json'
    }, function(response) {
      if(response.success) {
        loading.hide();
        if('area' in response.data.party) {
          if(response.data.party.area instanceof Array) {
            var items = [];
            response.data.party.area.forEach(function(area, i) {
              items.push({
                title: area.name,
                id: i
              });
            });
            // Construct Menu to show to user
            var areaMenu = new UI.Menu({
              sections: [{
                title: response.data.party.name,
                items: items
              }]
            });

            // Show the Menu, hide the loader
            areaMenu.show();

            areaMenu.on('select', function(e) {
              showLineup(response.data.party.area[e.item.id].lineup, response.data.party.name);
            });
          }
          else {
            showLineup(response.data.party.area.lineup, response.data.party.name);
          }
        }
        else {
          var card = new UI.Card({
            scrollable: true,
            title: response.data.party.name,
            body: 'No lineup published for this party!',
            style: 'large',
            backgroundColor: (current_watch.platform !== 'aplite' ? 'darkCandyAppleRed' : 'white'),
            titleColor: (current_watch.platform !== 'aplite' ? 'white' : 'black'),
            bodyColor: (current_watch.platform !== 'aplite' ? 'white' : 'black')
          });
          card.show();
        }
      }
      else {
        loading.hide();
        error.body('Error loading data, backend might be experiencing problems!');
        error.show();
      }
    }, function(err) {
      loading.hide();
      error.body('Error loading data, backend might be experiencing problems!');
      error.show();
    });
  },
  loadAll: function(splash, loading, error, latitude, longitude) {
    var _this = this;
    var params = '?radius=50';
    if(latitude && longitude) {
      params += '&latitude=' + latitude + '&longitude=' + longitude;
    }
    
    splash.hide();
    loading.show();

    var url = 'http://festiwatch.pebble.sillevis.net/' + params;
    ajax({
      url: url,
      method: 'GET',
      type: 'json'
    }, function(response) {
      if(response.success && response.data.length > 0) {
        loading.hide();

        var items = [];
        for (var i = 0 ; i < response.data.length ; i++) {
          items.push({
            title: response.data[i].name,
            subtitle: response.data[i].location + ': ' + response.data[i].visitors + ' visitors',
            id: response.data[i].id
          });
        }
        // Construct Menu to show to user
        var resultsMenu = new UI.Menu({
          sections: [{
            title: 'Nearby parties',
            items: items
          }]
        });

        // Show the Menu, hide the loader
        resultsMenu.show();

        resultsMenu.on('select', function(e) {
          loading.show();
          return _this.loadSingle(loading, error, e.item.id);
        });
      }
      else {
        loading.hide();
        error.body('Error loading data, malformed response from backend!');
        error.show();
      }
    }, function(err) {
      loading.hide();
      error.body('Error loading data, backend might be experiencing problems!');
      error.show();
    });
  }
};

module.exports = provider;