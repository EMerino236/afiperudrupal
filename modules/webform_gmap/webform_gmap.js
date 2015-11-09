Drupal.gmap.addHandler('gmap', function (elem) {
  var obj = this;

  obj.bind("addmarker", function (marker) {
    GEvent.addListener(marker.marker, 'dragend', function (latlng) {
      var prefix = "edit-submitted-" + marker.name + "-";
      jQuery ("#" + prefix + "latitude").val (latlng.lat());
      jQuery ("#" + prefix + "longitude").val (latlng.lng());
    });

  });

});
