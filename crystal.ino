#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>

#include "src/lumos-arduino/lumos-arduino/Logger.h"
#include "src/lumos-arduino/lumos-arduino/Patterns.h"

// Secrets are defined in another file called "secrets.h" to avoid commiting secrets
// into a public repo. You will need to change the secret values in secrets.h to
// connect your device to your network.
#include "secrets.h"

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
//Lumos lumos(PIXELS_COUNT, PIXELS_PIN);
//CrystalLight crystalLight(lumos.getStrip(), pixels);
//Flame flame(lumos.getStrip(), pixels);
//Rotate rotate(lumos.getStrip(), 5, RIGHT);

ModelRunner modelRunner;

void setup() {
  Serial.begin(115200);
  Serial.println("");

  setupNetwork();

  logServer.begin();

  strip.begin();
  strip.setBrightness(255);
  strip.show(); // Initialize all pixels to 'off'

//  lumos.runForever(&crystalLight);
}

void loop() {
  // Check for network activity.
  loopNetwork();

  // Animate the LEDs. 
  //  lumos.loop();
  modelRunner.loop(PIXELS_COUNT, pixels);
  Patterns::applyPixels(strip, pixels);
  strip.show();

  loopLogger();
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
