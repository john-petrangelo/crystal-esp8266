var defaultColor = "#e6e6fa";

window.addEventListener("load", startup, false);

function startup() {
    var brightness = document.querySelector("#brightness")
    fetch("/brightness", {method:'GET'})
      .then(response => response.json())
      .then(data => { 
          brightness.value = data.value;
          brightness.style.visibility = "visible";
        });
    brightness.addEventListener("input", brightnessDidChange);

    // Setup solid color picker - demo only, to be repurposed
    var colors = document.querySelector("#colors");
    colors.value = defaultColor;
    colors.addEventListener("change", colorDidChange);

    var home = document.getElementById("home");
    var crystal_modal = document.getElementById("crystal-modal");
    var crystal_button = document.getElementById("crystal-button");
    var close_button = document.getElementsByClassName("modal-back")[0];
    crystal_button.onclick = function() {
        home.classList.add("home-out");
        crystal_modal.classList.add("modal-open");
    }
    
    close_button.onclick = function() {
        home.classList.remove("home-out");
        crystal_modal.classList.remove("modal-open");
    }
}

function brightnessDidChange(event) {
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

function colorDidChange(event) {
    url = "/solid?color=" + event.target.value.substring(1);
    fetch(url, {method:'GET'});
}
