#include <ESP8266WebServer.h>
#include <LittleFS.h>

#include <Adafruit_NeoPixel.h>

#include "src/lumos-arduino/lumos-arduino/Colors.h"
#include "src/lumos-arduino/lumos-arduino/Logger.h"

#include "src/Animations.h"
#include "src/Model.h"
#include "src/Renderer.h"

String hostname = "crystal";
ESP8266WebServer server(80);

// Configuration information about the NeoPixel strip we are using.
int const PIXELS_COUNT = 24;
int8_t const PIXELS_PIN = 2;
neoPixelType const STRIP_FLAGS = NEO_GRB + NEO_KHZ800;
Adafruit_NeoPixel strip(PIXELS_COUNT, PIXELS_PIN, STRIP_FLAGS);

Color pixels[PIXELS_COUNT];

long const logDurationIntervalMS = 5000;
Renderer renderer;

void setup() {
  Serial.begin(115200);
  Serial.println("");

  if (!LittleFS.begin()) {
    Serial.println("Failed to start LittleFS");
  }

  setupNetwork();

  strip.begin();
  strip.setBrightness(255);
  strip.show(); // Initialize all pixels to 'off'

  renderer.setModel(makeDarkCrystal());
}

void loop() {
  static int lastUpdateMS = millis();

  long beforeMS = millis();
  
  // Check for network activity.
  loopNetwork();

  // Animate the LEDs. 
  renderer.loop(strip.numPixels(), [&strip](int i, Color color) { strip.setPixelColor(i, color); });
  strip.show();

  loopLogger();

  if (beforeMS - lastUpdateMS >= logDurationIntervalMS) {
    lastUpdateMS = beforeMS;
    long afterMS = millis();
    Logger::logf("%0.3f Completed loop, duration %dms, free heap %d bytes\n", afterMS / 1000.0, afterMS - beforeMS, ESP.getFreeHeap());
  }
}
