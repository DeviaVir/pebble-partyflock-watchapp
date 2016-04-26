'use strict';

var errorCard = require('Window/Error.js');
var loadingCard = require('Window/Loading.js');
var provider = require('DataProvider/FestiWatch.js');
var current_watch = require('./WatchInfo.js');

var UI = require('ui');
var Settings = require('settings');

var locationOptions = {
  'timeout': 15000,
  'maximumAge': 60000
};

function boot() {
  var card = new UI.Card({
    scrollable: true,
    title: 'FestiWatch',
    banner: 'IMAGE_PEOPLE',
    body: 'Please hold on while we get your location to show you parties nearby.',
    style: 'small',
    backgroundColor: (current_watch.platform !== 'aplite' ? 'darkCandyAppleRed' : 'white'),
    titleColor: (current_watch.platform !== 'aplite' ? 'white' : 'black'),
    bodyColor: (current_watch.platform !== 'aplite' ? 'white' : 'black')
  });
  card.show();

  navigator.geolocation.getCurrentPosition(function(pos) {
    if(pos && 'coords' in pos && 
        typeof pos.coords === 'object' &&
        'latitude' in pos.coords &&
        'longitude' in pos.coords &&
        pos.coords.latitude &&
        pos.coords.longitude) {
      return provider.loadAll(card, loadingCard, errorCard, pos.coords.latitude, pos.coords.longitude);
    }
    else {
      return provider.loadAll(card, loadingCard, errorCard);
    }
  }, function() {
    return provider.loadAll(card, loadingCard, errorCard);
  }, locationOptions);
}

console.log('Starting app', JSON.stringify(Settings.option()));
boot();
