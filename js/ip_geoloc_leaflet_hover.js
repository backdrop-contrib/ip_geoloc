
jQuery(document).bind('leaflet.feature', function(event, lFeature, feature) {
  // lFeature is the Leaflet feature just added to the map, eg a polygon.
  // feature.feature_id is the node ID, as set by ip_geoloc_plugin_style_leaflet.inc
  // lFeature.feature_id is used in the leaflet.map "hook" below, which is
  // called once the map is complete.
  if (feature.feature_id) {
    lFeature.feature_id = feature.feature_id;
  }
});

jQuery(document).bind('leaflet.map', function(event, map, lMap) {

  function filterHTML(id) {
    const html =
    `<filter id="${id}">
       <feOffset in="SourceAlpha" result="offOut" dx="0" dy="0"/>
       <feGaussianBlur in="offOut" result="blurOut" stdDeviation="0">
         <animate id="blur-on-${id}"  attributeName="stdDeviation" to="4" begin="indefinite" dur="0.40s" fill="freeze"/>
         <animate id="blur-off-${id}" attributeName="stdDeviation" to="0" begin="indefinite" dur="0.25s" fill="freeze"/>
       </feGaussianBlur>
       <feBlend in="SourceGraphic" in2="blurOut" mode="normal"/>
     </filter>`;
    return html
  }

  let layer = null;

  if (map.settings.openBalloonsOnHover || map.settings.polygonAddShadowOnHover || map.settings.polygonFillOpacityOnHover) {
    // Handle mouseover events.
    for (var leaflet_id in lMap._layers) {
      layer = lMap._layers[leaflet_id];
      if (layer._path) {

        if (map.settings.polygonAddShadowOnHover) {
          let svgs = document.getElementsByTagName('svg')
          if (svgs.length) {
            // Create a filter <def> for this polygon, if there isn't one.
            let defs = document.getElementsByTagName('defs');
            if (!defs.length) {
              svgs[0].insertAdjacentHTML('afterbegin', '<defs></defs>');
              defs = document.getElementsByTagName('defs');
            }
            defs[0].insertAdjacentHTML('beforeend', filterHTML('f' + layer.feature_id));

            // Link the path to the filter def.
            layer._path.setAttribute('filter', 'url(#f' + layer.feature_id + ')');
            // Set id on path for use below
            layer._path.setAttribute('id', 'p' + layer.feature_id);
          }
        }
        // Store the current fill opacity, so we can revert to it on mouseout.
        layer._fillOpacity = layer._path.getAttribute('fill-opacity');
        layer._strokeWidth = layer._path.getAttribute('stroke-width');
      }

      layer.on('mouseover', function(e) {
        if (map.settings.openBalloonsOnHover) {
          this.openPopup();
        }
        if (map.settings.polygonAddShadowOnHover) {
          const blur = document.getElementById("blur-on-f" + this.feature_id)
          if (blur) {
            const gs = document.getElementsByTagName("g")
            if (gs) {
               const path = document.getElementById("p" + this.feature_id);
              // appendChild() is so that the hovered element is on top and
              // will show shadow on all sides.
              gs[0].appendChild(path)
            }
            blur.beginElement();
          }
        }
        // setStyle is only available on some feature types.
        if (map.settings.polygonFillOpacityOnHover && typeof(this.setStyle) == 'function') {
          this.setStyle({ fillOpacity: map.settings.polygonFillOpacityOnHover })
          if (map.settings.polygonLineWeightOnHover) {
            this.setStyle({ weight: map.settings.polygonLineWeightOnHover })
          }
        }
      })
    }
  }

  if (map.settings.polygonAddShadowOnHover || map.settings.polygonFillOpacityOnHover) {
    // Handle mouseout events.
    for (var leaflet_id in lMap._layers) {
      layer = lMap._layers[leaflet_id];

      layer.on('mouseout', function(e) {

        if (map.settings.polygonAddShadowOnHover) {
          const blur = document.getElementById("blur-off-f" + this.feature_id);
          if (blur) blur.beginElement();
        }
        if (typeof(this.setStyle) == 'function') {
          this.setStyle({
            fillOpacity: this._fillOpacity,
            weight: this._strokeWidth,
          });
        }
      })
    }
  }

});