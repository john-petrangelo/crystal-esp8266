var defaultColor = "#e6e6fa";

window.addEventListener("load", startup, false);

function startup() {
    var brightness = document.getElementById("brightness")
    var brightnessBox = document.getElementById("brightness-box")
    fetch("/brightness")
      .then(response => {
        if (!response.ok) {
          throw Error(response.statusText);
        }
        return response.json();
      })
      .then(data => { 
          brightness.value = data.value;
          brightnessBox.style.visibility = "visible";
      })
      .catch(error => {
          brightness.value = 255;
          brightnessBox.style.visibility = "visible";
      });
    brightness.addEventListener("input", brightnessDidChange);

    // Setup solid color picker - demo only, to be repurposed
    let colors = document.querySelector("#colors");
    colors.value = defaultColor;
    colors.addEventListener("change", colorDidChange);

    let home = document.getElementById("home");
    let subpages = document.getElementsByClassName("subpage");
    let subpageTopBarTemplate = document.getElementById("subpage-top-bar-template");

    for (let subpage of subpages) {
        let clone = subpageTopBarTemplate.content.cloneNode(true);
        subpage.insertBefore(clone, subpage.firstChild);

        let label = subpage.querySelector(".subpage-title");
        label.textContent = subpage.dataset.title;
    
        let button = document.getElementById(subpage.dataset.buttonid);

        button.onclick = function() {
            home.classList.add("homepage-out");
            subpage.classList.add("subpage-is-open");
        }

        let back_button = subpage.getElementsByClassName("subpage-back")[0];
        back_button.onclick = function() {
            home.classList.remove("homepage-out");
            subpage.classList.remove("subpage-is-open");
        }
    }

    let colorSpeeds = document.getElementsByClassName("color-speed");
    let colorSpeedTemplate = document.getElementById("color-speed-template");
    for (let colorSpeed of colorSpeeds) {
        let clone = colorSpeedTemplate.content.cloneNode(true);
        colorSpeed.appendChild(clone);
        colorSpeed.querySelector("span").textContent = colorSpeed.dataset.title;

        colorSpeed.querySelector("input[type='color'").id = colorSpeed.id + "-color";
        colorSpeed.querySelector("input[type='range'").id = colorSpeed.id + "-speed";
        colorSpeed.addEventListener("input", crystalDidChange);
    }
}

function snapMin(value, minAllowed) {
    if (value < minAllowed) {
        value = Math.floor(2 * value / minAllowed) * minAllowed;
    }

    return value;
}

function brightnessDidChange(event) {
    event.target.value = snapMin(event.target.value, 40);
    url = "/brightness?value=" + event.target.value;
    fetch(url, {method:'PUT'});
}

// TODO Obsolete?
function colorDidChange(event) {
    let url = "/solid?color=" + event.target.value.substring(1);
    fetch(url, {method:'GET'});
}

var crystalData = {
    upper: {
        color: "ff00d0",
        speed: 5000
    },
    middle: {
        color: "ff00d0",
        speed: 8000
    },
    lower: {
        color: "ff00d0",
        speed: 7000
    }
};

function crystalDidChange(event) {
    switch (event.target.id) {
        case "crystal-upper-color":
            crystalData.upper.color = event.target.value.substring(1);
            break;
        case "crystal-upper-speed":
            event.target.value = snapMin(event.target.value, 1000);
            crystalData.upper.speed = event.target.value;
            break;
        case "crystal-middle-color":
            crystalData.middle.color = event.target.value.substring(1);
            break;
        case "crystal-middle-speed":
            event.target.value = snapMin(event.target.value, 1000);
            crystalData.middle.speed = event.target.value;
            break;
        case "crystal-lower-color":
            crystalData.lower.color = event.target.value.substring(1);
            break;
        case "crystal-lower-speed":
            event.target.value = snapMin(event.target.value, 1000);
            crystalData.lower.speed = event.target.value;
            break;
    }

    fetch('/crystal', {method: 'PUT', body: JSON.stringify(crystalData)});
}

function setCrystal(color) {
    let colorInputs = document.querySelectorAll(".color-speed-container > input[type='color'");
    for (let colorInput of colorInputs) {
        colorInput.value = "#" + color;
    }

    crystalData.upper.color = color;
    crystalData.middle.color = color;
    crystalData.lower.color = color;

    fetch('/crystal', {method: 'PUT', body: JSON.stringify(crystalData)});
}
