
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
  // Note that the off-transition is set to half the duration of the on-
  // transition.
  // You may specify up to 4 animation effects, with the parent element, e.g.
  // the <feGaussianBlur>, called effect#-ID, and the associated animations
  // called effect#-onID and effect#-offID, where # is a digit 1..4 and ID is
  // the unique ID used in both <filter id="fID"> and <path id="pID">.
  // The ID is in fact the Drupal node ID.
  function filterHTML(id, duration, effect) {
    if (effect == 'surge') {
      //    Filter primitive "feOffset" by itself was not necessary to obtain the alpha channel so has been taken out.
      // 1) Filter primitive "feGaussianBlur" takes the alpha channel of the source graphic as its input and applies an animated standard deviation blur.
      // 2) Filter primitive "feComponentTransfer" takes the previous blur as its input and uses the feFuncA filter primitive to remap its alpha pixels according to the specified table values.
      //    This is output into a new buffer called "offsetmorph."
      // 3) Filter primitive "feFlood" fills the entire filter subregion with the color and opacity defined. This results in a square color overlay.
      // 4) Filter primitive "feComposite" masks the last two primitives in image space using the "in" Porter-Duff compositing operation. This is output into a new buffer called "surgeShadow."
      //    Three additional Filter primitives: feGaussianBlur, feFlood, & feComposite have been taken out as they together created an additional and unnecessary second blur effect.
      // 5) Filter primitive "feMerge" applies the surgeShadow filter result and the original source graphic in front.
      let html =
        `<filter id="f${id}" width="200%" height="200%" x="-50%" y="-50%">
          <feGaussianBlur id="effect1-${id}" in="SourceAlpha" result="blurOut" stdDeviation="0">`
        if (!useTween) {
          html +=
            `  <animate id="effect1-on${id}"  attributeName="stdDeviation" to="4" begin="indefinite" dur="${duration}s" fill="freeze"/>
               <animate id="effect1-off${id}" attributeName="stdDeviation" to="0" begin="indefinite" dur="${duration/2}s" fill="freeze"/>`
        }
        html +=
        ` </feGaussianBlur>
          <feComponentTransfer in="blurOut" result="offsetmorph">
            <feFuncA type="table" tableValues="0 .05 1 1 1 1 1 1 .5 0 0 0 0 0 0 0 0 .01 0"/>
          </feComponentTransfer>
          <feFlood id="effect2-${id}" flood-color="black" flood-opacity="0">`
        if (!useTween) {
          html +=
            `  <animate id="effect2-on${id}"  attributeName="flood-opacity" to="0.45" begin="indefinite" dur="${duration}s" fill="freeze"/>
               <animate id="effect2-off${id}" attributeName="flood-opacity" to="0.00" begin="indefinite" dur="${duration/2}s" fill="freeze"/>`
        }
        html +=
         ` </feFlood>
          <feComposite operator="in" in2="offsetmorph" result="surgeShadow"/>
          <feMerge>
            <feMergeNode in="surgeShadow"></feMergeNode>
            <feMergeNode in="SourceGraphic"></feMergeNode>
          </feMerge>
        </filter>`;
        return html;
    }
    // Default to simple Gaussian blur shadow.
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
    // Store the current fill opacity and stroke so we can revert on mouseout.
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
            TweenMax.to('#effect2-' + id, duration, { attr: { 'flood-opacity': 0.45 }, paused: true })
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

  function resetPolygonAttributes(layer) {
    if (layer && typeof(layer.setStyle) == 'function') {
      layer.setStyle({
        fillOpacity: layer._fillOpacity,
        weight     : layer._strokeWidth,
      })
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

    // Pure HTML: allow up to 4 animations.
    for (let i = 1; i <= 4; i++) {
      const effect = document.getElementById(`effect${i}-` + (on ? 'on' : 'off') + layer.feature_id);
      if (effect) effect.beginElement();
    }
  }

  function handlePolygonMouseOver(layer) {

    if (layer == activeFeature) return;

    if (map.settings.polygonFillOpacityOnHover) {
      // Reset style on the previously active polygon, if still lingering,
      // e.g. when we've hovered in from a balloon that overlaps 2 polygons.
      resetPolygonAttributes(activeFeature)

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
      gs[0].appendChild(document.getElementById("p" + layer.feature_id))
      // Apply the effects animation.
      effects(true, layer)
    }
    activeFeature = layer
  }

  function handlePolygonMouseOut(layer, related) {
    // If there is no activeFeature, we have nothing to deactivate.
    if (!activeFeature) return

    // A mouse-out with an empty "related.id" usually indicates a mouse-over
    // on a marker surrounded by the polygon the mouse is in. Or an open popup
    // overlapping with the polygon. We ignore such spurious mouse-outs.
    // Note that the "map" itself DOES have a related.id that starts with
    // "ip-geoloc-map-..."
    if (!related || !related.id) return

    if (map.settings.polygonFillOpacityOnHover) {
      resetPolygonAttributes(activeFeature)
    }
    if (map.settings.polygonAddShadowOnHover) {
      effects(false, activeFeature)
    }
    activeFeature = null
  }

  // Loop through the map layers, initialise each and assign hover handlers.
  for (let leaflet_id in lMap._layers) {
    let layer = lMap._layers[leaflet_id]
    if (!layer._path) continue;

    if (map.settings.polygonFillOpacityOnHover || map.settings.polygonAddShadowOnHover) {
      // Set ids on polygons and assign animation.
      prepareLayer(layer);

      // Handle mouseouts
      layer.on('mouseout', function(event) {
        handlePolygonMouseOut(this, event.originalEvent.relatedTarget)
      });
    }
    // Handle mouseovers
    layer.on('mouseover', function(event) {
      if (map.settings.openBalloonsOnHover) {
        this.openPopup()
      }
      if (map.settings.polygonFillOpacityOnHover || map.settings.polygonAddShadowOnHover) {
        handlePolygonMouseOver(this)
      }
    })
  }

  // Safety net: when pointer moves out from map, terminate active hightlights.
  lMap.on('mouseout', function(event) {
    // Remove fill opacity and blur/shadow, if still lingering.
    if (map.settings.polygonFillOpacityOnHover) {
      resetPolygonAttributes(activeFeature)
    }
    if (map.settings.polygonAddShadowOnHover) {
      effects(false, activeFeature)
    }
    activeFeature = null
  })
})
