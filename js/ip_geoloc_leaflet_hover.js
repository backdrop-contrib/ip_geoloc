
jQuery(document).bind('leaflet.feature', function(event, lFeature, feature) {
  // lFeature is the Leaflet feature just added to the map, eg a polygon.
  // feature.feature_id is the node ID, as set by ip_geoloc_plugin_style_leaflet.inc
  // lFeature.feature_id is used in the leaflet.map "hook" below, which is
  // called once the map is complete.
  if (feature.feature_id) {
    lFeature.feature_id = feature.feature_id
  }
})

jQuery(document).bind('leaflet.map', function(event, map, lMap) {

  function filterHTML(id) {
    const html =
    `<filter id="${id}">
       <feOffset in="SourceAlpha" result="offOut" dx="0" dy="0"/>
       <feGaussianBlur in="offOut" result="blurOut" stdDeviation="0">
         <animate id="blur-on-${id}"  attributeName="stdDeviation" to="7" begin="indefinite" dur="0.40s" fill="freeze"/>
         <animate id="blur-off-${id}" attributeName="stdDeviation" to="0" begin="indefinite" dur="0.25s" fill="freeze"/>
       </feGaussianBlur>
       <feBlend in="SourceGraphic" in2="blurOut" mode="normal"/>
     </filter>`;
    return html
  }

  let activeFeature = null;

  if (map.settings.openBalloonsOnHover || map.settings.polygonFillOpacityOnHover || map.settings.polygonAddShadowOnHover) {

    // Handle mouseover events.
    for (let leaflet_id in lMap._layers) {
      let layer = lMap._layers[leaflet_id]

      // The check for ._path excludes markers (and circles ?)
      if (layer._path) {
        // Set id on path for use in 'mouseover' handling, below.
        layer._path.setAttribute('id', 'p' + layer.feature_id)

        // Store the current fill opacity, so we can revert to it on mouseout.
        layer._fillOpacity = layer._path.getAttribute('fill-opacity')
        layer._strokeWidth = layer._path.getAttribute('stroke-width')

        if (map.settings.polygonAddShadowOnHover && layer.feature_id) {
          let svgs = document.getElementsByTagName('svg')
          if (svgs.length) {
            // Create a filter <def> for this polygon, if there isn't one.
            let defs = document.getElementsByTagName('defs');
            if (!defs.length) {
              svgs[0].insertAdjacentHTML('afterbegin', '<defs></defs>')
              defs = document.getElementsByTagName('defs')
            }
            defs[0].insertAdjacentHTML('beforeend', filterHTML('f' + layer.feature_id))

            // Link the path to the filter def.
            layer._path.setAttribute('filter', 'url(#f' + layer.feature_id + ')')
          }
        }
      }

      layer.on('mouseover', function(e) {
        if (map.settings.openBalloonsOnHover) {
          this.openPopup()
        }
        if (!e.originalEvent.toElement.id) {
          return
        }
        // Don't do anything when returning to ourselves while already blurred.
        if (activeFeature && (e.originalEvent.toElement.id == 'p' + activeFeature.feature_id)) {
          return
        }

        if (map.settings.polygonFillOpacityOnHover && typeof(this.setStyle) == 'function') {
          if (activeFeature && (activeFeature != this)) {
            // Reset opacity from previous feature, if still lingering
            activeFeature.setStyle({
              fillOpacity: activeFeature._fillOpacity,
              weight: activeFeature._strokeWidth,
            })
          }
          this.setStyle({ fillOpacity: map.settings.polygonFillOpacityOnHover })
          if (map.settings.polygonLineWeightOnHover) {
            this.setStyle({ weight: map.settings.polygonLineWeightOnHover })
          }
        }

        if (map.settings.polygonAddShadowOnHover) {
          if (activeFeature) {
            // Remove blur from previous feature, if still lingering.
            const blurOff = document.getElementById("blur-off-f" + activeFeature.feature_id);
            if (blurOff) blurOff.beginElement();
          }
          const blur = document.getElementById("blur-on-f" + this.feature_id)
          if (blur) {
            const gs = document.getElementsByTagName("g")
            if (gs) {
               const path = document.getElementById("p" + this.feature_id)
              // appendChild() is so that the hovered element is on top and
              // will show shadow on all sides.
              gs[0].appendChild(path)
            }
            blur.beginElement()
          }
        }

        activeFeature = this
      })
    }

    // Safety net: when pointer moves out from map, terminate active decoration.
    lMap.on('mouseout', function(e) {
       if (!activeFeature) return;

       // Remove fill opacity and blur/shadow, if still lingering.
       if (map.settings.polygonFillOpacityOnHover) {
          activeFeature.setStyle({
            fillOpacity: activeFeature._fillOpacity,
            weight: activeFeature._strokeWidth,
          })
       }
       if (map.settings.polygonAddShadowOnHover) {
         const blurOff = document.getElementById("blur-off-f" + activeFeature.feature_id)
         if (blurOff) blurOff.beginElement()
       }
       activeFeature = null
    });
  }

  if (map.settings.polygonFillOpacityOnHover || map.settings.polygonAddShadowOnHover) {
    // Handle mouseout events.
    for (let leaflet_id in lMap._layers) {
      let layer = lMap._layers[leaflet_id]

      layer.on('mouseout', function(e) {
        // Only respond when a hover-out occurs on a feature we've given an ID
        if (!e.originalEvent.toElement || !e.originalEvent.toElement.id) {
          return
        }
        // Don't do anything when returning to ourselves while still active.
        if (!activeFeature || (e.originalEvent.toElement.id == 'p' + activeFeature.feature_id)) {
          return
        }

        if (map.settings.polygonFillOpacityOnHover && typeof(this.setStyle) == 'function') {
          this.setStyle({
            fillOpacity: this._fillOpacity,
            weight: this._strokeWidth,
          })
        }

        if (map.settings.polygonAddShadowOnHover) {
          const blur = document.getElementById("blur-off-f" + activeFeature.feature_id)
          if (blur) blur.beginElement()
        }

        activeFeature = null
      })
    }
  }

})
