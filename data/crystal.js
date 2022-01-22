var defaultColor = "#e6e6fa";

window.addEventListener("load", startup, false);

function startup() {
    var brightness = document.querySelector("#brightness");
    brightness.value = 255;
    brightness.addEventListener("input", changeBrightness);

    // Setup solid color picker - demo only, to be repurposed
    var colors = document.querySelector("#colors");
    colors.value = defaultColor;
    colors.addEventListener("change", changeColor);
}

function changeBrightness(event) {
    minAllowed = 40;
    if (event.target.value < minAllowed) {
        if (event.target.value > minAllowed/2) {
            event.target.value = minAllowed;
        } else {
            event.target.value = 0;
        }
    }

    url = "/brightness?value=" + event.target.value;
    fetch(url, {method:'PUT'});
  }

function changeColor(event) {
    url = "/solid?color=" + event.target.value.substring(1);
    fetch(url, {method:'GET'});
  }
