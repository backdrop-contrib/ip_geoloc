
jQuery(document).bind('leaflet.feature', function(event, lFeature, feature) {
  // lFeature is the Leaflet feature just added to the map, eg a polygon.
  // feature.feature_id is the node ID, as set by ip_geoloc_plugin_style_leaflet.inc
  // lFeature.feature_id is used in the leaflet.map "hook" below, which is
  // called once the map is complete.
  if (feature.feature_id && !lFeature.feature_id) {
    lFeature.feature_id = feature.feature_id
  }
})

jQuery(document).bind('leaflet.map', function(event, map, lMap) {

  const useTween = map.settings.useTweenMaxForShadowOnHover;
  let activeFeature = null;

  function filterHTML(id) {
    let html =
      `<filter id="f${id}">
         <feOffset in="SourceAlpha" result="offOut" dx="0" dy="0"/>
         <feGaussianBlur id="blur${id}" in="offOut" result="blurOut" stdDeviation="0">`
    if (!useTween) {
      html +=
        `  <animate id="blur-on${id}"  attributeName="stdDeviation" to="7" begin="indefinite" dur=".45s" fill="freeze"/>
           <animate id="blur-off${id}" attributeName="stdDeviation" to="0" begin="indefinite" dur=".25s" fill="freeze"/>`
    }
    return html +
      `  </feGaussianBlur>
         <feBlend in="SourceGraphic" in2="blurOut" mode="normal"/>
     </filter>`;
  }

  function prepareLayer(layer) {
    // The check for ._path excludes markers (and circles ?)
    if (!layer._path) return
    // Store the current fill opacity, so we can revert to it on mouseout.
    layer._fillOpacity = layer._path.getAttribute('fill-opacity')
    layer._strokeWidth = layer._path.getAttribute('stroke-width')

    const id = layer.feature_id;
    if (!id) return;

    // Set id on path for use in 'mouseover' handling, below.
    layer._path.setAttribute('id', 'p' + id)

    if (!map.settings.polygonAddShadowOnHover) return;

    let svgs = document.getElementsByTagName('svg')
    if (svgs.length) {
      // Create a <filter> inside <def> for this polygon, if there isn't one.
      let defs = document.getElementsByTagName('defs');
      if (!defs.length) {
        svgs[0].insertAdjacentHTML('afterbegin', '<defs></defs>')
        defs = document.getElementsByTagName('defs')
      }
      defs[0].insertAdjacentHTML('beforeend', filterHTML(id))

      // Link the path to the filter def.
      layer._path.setAttribute('filter', `url(#f${id})`)

      if (useTween) {
        // Attach TweenMax animation, as opposed to using HTML5 <animate> tag.
        layer.tweenMaxAnimation = TweenMax.to('#blur' + id, 0.45, { attr: { 'stdDeviation': 7 }, paused: true });
      }
    }
  }

  function handlePolygonMouseOver(layer, toElement) {

    if (!toElement.id) {
      return
    }
    // Don't do anything when returning to ourselves while already blurred.
    if (activeFeature && (toElement.id == 'p' + activeFeature.feature_id)) {
      return
    }

    if (map.settings.polygonFillOpacityOnHover && typeof(layer.setStyle) == 'function') {
      if (activeFeature && (activeFeature != layer)) {
        // Reset opacity from previous feature, if still lingering
        activeFeature.setStyle({
          fillOpacity: activeFeature._fillOpacity,
          weight     : activeFeature._strokeWidth,
        })
      }
      layer.setStyle({ fillOpacity: map.settings.polygonFillOpacityOnHover })
      if (map.settings.polygonLineWeightOnHover) {
        layer.setStyle({ weight: map.settings.polygonLineWeightOnHover })
      }
    }

    if (map.settings.polygonAddShadowOnHover) {
      // Remove blur from previous feature, if still lingering.
      if (activeFeature) {
        if (activeFeature.tweenMaxAnimation) {
          activeFeature.tweenMaxAnimation.reverse();
        }
        else {
          const blurOff = document.getElementById("blur-off" + activeFeature.feature_id);
          if (blurOff) blurOff.beginElement();
        }
      }

      // appendChild() so that the hovered element is on top and will show
      // blur/shadow effect on all sides.
      const gs = document.getElementsByTagName("g")
      if (gs) {
        const path = document.getElementById("p" + layer.feature_id)
        gs[0].appendChild(path)
      }

      if (layer.tweenMaxAnimation) {
        layer.tweenMaxAnimation.play();
      }
      else {
        const blur = document.getElementById("blur-on" + layer.feature_id)
        if (blur) blur.beginElement()
      }
    }

    activeFeature = layer
  }

  function handlePolygonMouseOut(layer, toElement) {
    // Only respond when a hover-out occurs on a feature we've given an ID
    if (!toElement || !toElement.id) {
      return
    }
    // Don't do anything when returning to ourselves while still active.
    if (!activeFeature || (toElement.id == 'p' + activeFeature.feature_id)) {
      return
    }

    if (map.settings.polygonFillOpacityOnHover) {
      activeFeature.setStyle({
        fillOpacity: activeFeature._fillOpacity,
        weight     : activeFeature._strokeWidth,
      })
    }

    if (map.settings.polygonAddShadowOnHover) {
      if (activeFeature.tweenMaxAnimation) {
        activeFeature.tweenMaxAnimation.reverse();
      }
      else {
        const blur = document.getElementById("blur-off" + activeFeature.feature_id)
        if (blur) blur.beginElement()
      }
    }

    activeFeature = null
  }

  // Loop through the map layers, initialise each and assign hover handlers.
  for (let leaflet_id in lMap._layers) {
    let layer = lMap._layers[leaflet_id]

    if (map.settings.polygonFillOpacityOnHover || map.settings.polygonAddShadowOnHover) {
      prepareLayer(layer);

      // Handle mouseouts
      layer.on('mouseout', function(e) {
        handlePolygonMouseOut(this, e.originalEvent.toElement)
      });
    }
    // Handle mouseovers
    layer.on('mouseover', function(e) {
      if (map.settings.openBalloonsOnHover) {
        this.openPopup()
      }
      if (map.settings.polygonFillOpacityOnHover || map.settings.polygonAddShadowOnHover) {
        handlePolygonMouseOver(this, e.originalEvent.toElement)
      }
    })
  }
/*
  jQuery("svg path").each(function(i, path) {

    const id = '#blur' + path.id.substr(1)
    var tweenMaxAnimation = TweenMax.to(id, 0.4, { attr: { 'stdDeviation': 4 }, paused: true });

    jQuery(path).hover(function() {
      jQuery(path).appendTo("svg g");
      tweenMaxAnimation.play()
    }, function() {
      tweenMaxAnimation.reverse()
    })
  });
*/
  // Safety net: when pointer moves out from map, terminate active decoration.
  lMap.on('mouseout', function(e) {
    // Take down popups here?

    if (!activeFeature) return;

    // Remove fill opacity and blur/shadow, if still lingering.
    if (map.settings.polygonFillOpacityOnHover) {
      activeFeature.setStyle({
        fillOpacity: activeFeature._fillOpacity,
        weight     : activeFeature._strokeWidth,
      })
    }
    if (map.settings.polygonAddShadowOnHover && !activeFeature.tweenMaxAnimation) {
      const blurOff = document.getElementById("blur-off-f" + activeFeature.feature_id)
      if (blurOff) blurOff.beginElement()
    }
    activeFeature = null
  })
})
