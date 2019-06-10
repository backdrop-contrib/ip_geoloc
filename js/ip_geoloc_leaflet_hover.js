
jQuery(document).bind('leaflet.feature', function(event, lFeature, feature) {
  // lFeature is the Leaflet feature just added to the map, eg a polygon.
  // feature.feature_id is the node ID, as set by ip_geoloc_plugin_style_leaflet.inc
  // lFeature.feature_id is used in the leaflet.map "hook" below, which is
  // called once the map is initialised.
  if (feature.feature_id && !lFeature.feature_id) {
    // Copy node ID.
    lFeature.feature_id = feature.feature_id
  }
})

/*
 * The reason why this code is quite long is because of quirky mouse-event
 * behaviour. When the mouse is moved onto a polygon and then onto a marker
 * inside the polygon, this emits a mouse-out on the polygon, causing it to
 * drop its blur/shadow effect. This is not what we want.
 * The code below detects when a received mouse-out isn't really a mouse-out
 * from the polygon and ignores it. It then removes the blur/shadow effect
 * when the mouse is moved IN over the next polygon. For this we keep track
 * of the "active" highlighted polygon in the variable activeFeature.
 */
jQuery(document).bind('leaflet.map', function(event, map, lMap) {

  const useTween = map.settings.useTweenMaxForShadowOnHover;
  let activeFeature = null;

  // Return the <filter> HTML for the shadow effect.
  // The default is a simple animated 'blur'.
  // The 'surge' effect was designed by Matt Winans for surgemovement.com
  // Note that the off-transition is half the duration of the on-transition.
  // You may specify up to 3 animation effects, with the parent element being
  // call effect1-ID, and the associated animations called effect1-onID and
  // effect1-offID, where ID is a unique (polygon) ID.
  function filterHTML(id, duration, effect) {
    if (effect == 'surge') {
      let html =
      `<filter id="f${id}" width="200%" height="200%"  x="-50%" y="-50%" filterRes="1000">
        <feOffset in="SourceAlpha" result="offOut" dx="0" dy="0"/>
        <feGaussianBlur id="effect1-${id}" in="offOut" result="blurOut" stdDeviation="0">`
      if (!useTween) {
        html +=
          `  <animate id="effect1-on${id}"  attributeName="stdDeviation" to="4" begin="indefinite" dur="${duration}s" fill="freeze"/>
             <animate id="effect1-off${id}" attributeName="stdDeviation" to="0" begin="indefinite" dur="${duration/2}s" fill="freeze"/>`
      }
      html +=
      ` </feGaussianBlur>

        <feComponentTransfer result="offsetmorph">
          <feFuncA type="table" tableValues="0 .05 1 1 1 1 1 1 .5 0 0 0 0 0 0 0 0 0.01 0"/>
        </feComponentTransfer>
        <feFlood id="effect2-${id}" flood-color="black" flood-opacity="0">`
      if (!useTween) {
        html +=
          `  <animate id="effect2-on${id}"  attributeName="flood-opacity" to="0.35" begin="indefinite" dur="${duration}s" fill="freeze"/>
             <animate id="effect2-off${id}" attributeName="flood-opacity" to="0.00" begin="indefinite" dur="${duration/2}s" fill="freeze"/>`
      }
      html +=
       ` </feFlood>

        <feComposite operator="in" in2="offsetmorph" result="stroke"/>
        <feGaussianBlur id="effect3-{id}" stdDeviation="0" result="offsetblur">`
      if (!useTween) {
        html +=
          `  <animate id="effect3-on${id}"  attributeName="stdDeviation" to="5" begin="indefinite" dur="${duration}s" fill="freeze"/>
             <animate id="effect3-off${id}" attributeName="stdDeviation" to="0" begin="indefinite" dur="${duration/2}s" fill="freeze"/>`
      }
      html +=
      ` </feGaussianBlur>

        <feFlood flood-color="black"/>
        <feComposite operator="in" in2="offsetblur" result="blur"/>
        <feMerge>
          <feMergeNode in="blur"></feMergeNode>
          <feMergeNode in="stroke"></feMergeNode>
          <feMergeNode in="SourceGraphic"></feMergeNode>
        </feMerge>
      </filter>`;
      return html;
    }
    // Default to simple Gaussian blur shaddow.
    let html =
      `<filter id="f${id}">
         <feOffset in="SourceAlpha" result="offOut" dx="0" dy="0"/>
         <feGaussianBlur id="effect1-${id}" in="offOut" result="blurOut" stdDeviation="0">`
    if (!useTween) {
      html +=
        `  <animate id="effect1-on${id}"  attributeName="stdDeviation" to="7" begin="indefinite" dur="${duration}s" fill="freeze"/>
           <animate id="effect1-off${id}" attributeName="stdDeviation" to="0" begin="indefinite" dur="${duration/2}s" fill="freeze"/>`
    }
    return html +
    ` </feGaussianBlur>
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
      const duration = 0.45
      defs[0].insertAdjacentHTML('beforeend', filterHTML(id, duration, map.settings.shadowOnHoverEffect))

      if (useTween) {
        // Attach TweenMax animation, as opposed to using HTML5 <animate> tag.
        if (map.settings.shadowOnHoverEffect == 'surge') {
          layer.tweenMaxAnimations = [
            TweenMax.to('#effect1-' + id, duration, { attr: { 'stdDeviation' : 4.00 }, paused: true }),
            TweenMax.to('#effect2-' + id, duration, { attr: { 'flood-opacity': 0.35 }, paused: true }),
            TweenMax.to('#effect3-' + id, duration, { attr: { 'stdDeviation' : 5.00 }, paused: true })
          ]
        }
        else {
          layer.tweenMaxAnimations = [ TweenMax.to('#effect1-' + id, duration, { attr: { 'stdDeviation': 4.0 }, paused: true }) ]
        }
      }
      // Link the path to the filter def.
      layer._path.setAttribute('filter', `url(#f${id})`)
    }
  }

  function effects(on, layer) {
    if (!layer) return;

    if (layer.tweenMaxAnimations) {
      // TweenMax implementation allows any number of animations
      layer.tweenMaxAnimations.forEach(animation => {
        if (on)
          animation.play();
        else
          animation.reverse();
      })
      return;
    }

    // Pure HTML: allow up to 4 animationa.
    for (let i = 1; i <= 4; i++) {
      const effect = document.getElementById(`effect${i}-` + (on ? 'on' : 'off') + layer.feature_id);
      if (effect) effect.beginElement();
    }
  }

  function handlePolygonMouseOver(layer, toElement) {

    if (!toElement.id) {
      return
    }
    // Don't do anything when returning to ourselves while still active.
    if (activeFeature && (toElement.id == 'p' + activeFeature.feature_id)) {
      return
    }

    if (map.settings.polygonFillOpacityOnHover && typeof(layer.setStyle) == 'function') {
      if (activeFeature && (activeFeature != layer)) {
        // Reset style on the previously active polygon, if still lingering.
        activeFeature.setStyle({
          fillOpacity: activeFeature._fillOpacity,
          weight     : activeFeature._strokeWidth,
        })
      }
      // Now set the highlight style for the hovered polygon.
      layer.setStyle({ fillOpacity: map.settings.polygonFillOpacityOnHover })
      if (map.settings.polygonLineWeightOnHover) {
        layer.setStyle({ weight: map.settings.polygonLineWeightOnHover })
      }
    }

    if (map.settings.polygonAddShadowOnHover) {
      // Remove effect(s) on previously highlighted polygon, if still lingering.
      effects(false, activeFeature);

      // appendChild() so that the hovered element is on top and will show
      // blur/shadow effect on all sides at all times.
      const gs = document.getElementsByTagName("g")
      if (gs) {
        const path = document.getElementById("p" + layer.feature_id)
        gs[0].appendChild(path)
      }

      // Apply the effect animation.
      effects(true, layer)
    }
    activeFeature = layer
  }

  function handlePolygonMouseOut(layer, toElement) {
    // Only respond when a hover-out occurs on a feature we've given an ID.
    // toElement==null when mousing out from border polygon and out of map.
    if (!toElement || !toElement.id || !activeFeature) {
      return
    }
    // Don't do anything when returning to ourselves while still active.
    if (toElement.id == 'p' + activeFeature.feature_id) {
      return
    }

    if (map.settings.polygonFillOpacityOnHover) {
      activeFeature.setStyle({
        fillOpacity: activeFeature._fillOpacity,
        weight     : activeFeature._strokeWidth,
      })
    }
    if (map.settings.polygonAddShadowOnHover) {
      effects(false, activeFeature)
    }
    activeFeature = null
  }

  // Loop through the map layers, initialise each and assign hover handlers.
  for (let leaflet_id in lMap._layers) {
    let layer = lMap._layers[leaflet_id]

    if (map.settings.polygonFillOpacityOnHover || map.settings.polygonAddShadowOnHover) {
      // Set ids on layers with _path (polygons) and assign animation.
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

  // Safety net: when pointer moves out from map, terminate active hightlights.
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
    if (map.settings.polygonAddShadowOnHover && !activeFeature.tweenMaxAnimations) {
      effects(false, activeFeature)
    }
    activeFeature = null
  })
})
