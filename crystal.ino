#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>

#include "src/lumos-arduino/lumos-arduino/Colors.h"
#include "src/lumos-arduino/lumos-arduino/Logger.h"

#include "src/Animations.h"
#include "src/Model.h"
#include "src/ModelRunner.h"

// Configuration information about the NeoPixel strip we are using.
int const PIXELS_COUNT = 24;
int8_t const PIXELS_PIN = 2;
neoPixelType const STRIP_FLAGS = NEO_GRB + NEO_KHZ800;
Adafruit_NeoPixel strip(PIXELS_COUNT, PIXELS_PIN, STRIP_FLAGS);

String hostname = "crystal";
ESP8266WebServer server(80);

// Server used for logging.
WiFiServer logServer(8000);
WiFiClient logClient;

Color pixels[PIXELS_COUNT];

long const logDurationIntervalMS = 1000;
ModelRunner modelRunner;

void setup() {
  Serial.begin(115200);
  Serial.println("");

  setupNetwork();

  logServer.begin();

  strip.begin();
  strip.setBrightness(255);
  strip.show(); // Initialize all pixels to 'off'

  modelRunner.setModel(makeCrystalPower());
}

void loop() {
  static int lastUpdateMS = millis();

  long beforeMS = millis();
  
  // Check for network activity.
  loopNetwork();

  // Animate the LEDs. 
  modelRunner.loop(strip.numPixels(), [&strip](int i, Color color) { strip.setPixelColor(i, color); });
  strip.show();

  loopLogger();

  if (beforeMS - lastUpdateMS >= logDurationIntervalMS) {
    lastUpdateMS = beforeMS;
    long afterMS = millis();
    Logger::logf("Completed loop, duration %dms\n", afterMS - beforeMS);
  }
}

// Check to see if the network logger needs to be setup or torn down
void loopLogger() {
  if (!logClient) {
    // No log client. Check the server for new clients.
    logClient = logServer.available();
    if (logClient) {
      // We've got a new log client.
      Logger::setStream(&logClient);
      Logger::logMsgLn("Connected to WiFi logging...");
    }
  }

  if (logClient && !logClient.connected()) {
    // Not connected anymore, switch logging back to serial.
    Logger::setStream(&Serial);
    logClient.stop();
  }
}
