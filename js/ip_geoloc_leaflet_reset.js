
// Inspired by Roger Codina and
// http://gis.stackexchange.com/questions/87199/how-does-one-reset-map-fitbounds-zoomtofeature-to-the-original-zoom

L.Control.Reset = L.Control.extend({

  options: {
    position: 'topleft'
  },

  onAdd: function (map) {
    map.on('viewreset', this.captureBounds, map);

    var button = L.DomUtil.create('a', 'leafet-control-reset leaflet-bar');
    if (map.options.resetControlCss) {
      L.DomUtil.addClass(button, map.options.resetControlCss);
    }
    else {
      button.innerHTML = 'R';
    }
    button.setAttribute('title', 'Reset'); 
    button.onclick = function() {
      map.fitBounds(map.initialBounds);
    };
    return button;
  },

  captureBounds: function (event) {
    if (!this.initialBounds) {
      this.initialBounds = event.target.getBounds();
    }
  }
});

L.Map.addInitHook(function () {
  this.addControl(new L.Control.Reset());
});
