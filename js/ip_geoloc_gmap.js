
/* Use this when you want multiple maps on the same page, e.g. via Views */

function displayGMap(latitude, longitude, elementId) {
  var mapOptions = {
    mapTypeId: google.maps.MapTypeId.ROADMAP,
    disableDefaultUI: true,
    zoom: 15,
    zoomControl: true
  };
  var map = new google.maps.Map(document.getElementById(elementId), mapOptions);
  var position = new google.maps.LatLng(latitude, longitude);
  map.setCenter(position);
  var marker = new google.maps.Marker({ map: map, position: position });
}