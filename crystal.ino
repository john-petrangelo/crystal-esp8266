#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Adafruit_NeoPixel.h>

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
//CrystalLight crystalLight(lumos.getStrip(), pixels);
//Flame flame(lumos.getStrip(), pixels);

ModelRunner modelRunner;

void setup() {
  Serial.begin(115200);
  Serial.println("");

  setupNetwork();

  logServer.begin();

  strip.begin();
  strip.setBrightness(255);
  strip.show(); // Initialize all pixels to 'off'

  Model *grad_left = new GradientModel("grad-left", RED, YELLOW);
  Model *grad_right = new GradientModel("grad-right", BLUE, GREEN);

  Model *rot_left = new RotateModel("rotate down", 4.0, RotateModel::DOWN, grad_left);
  Model *rot_right = new RotateModel("rotate up", 1.0, RotateModel::UP, grad_right);

  Model *map_left = new MapModel("map left", 0.0, 0.2, 0.0, 1.0, rot_left);
  Model *map_right = new MapModel("map right", 0.2, 1.0, 0.0, 1.0, rot_right);

  Model *window = new WindowModel("window", 0.0, 0.2, map_left, map_right);
  Model *rot_window = new RotateModel("rotate window", 0.5, RotateModel::DOWN, window);
  
  modelRunner.setModel(rot_window);
}

void loop() {
  static int lastUpdateMS = millis();
  long const logDurationIntervalMS = 1000;

  long now = millis();
  
  // Check for network activity.
  loopNetwork();

  // Animate the LEDs. 
  modelRunner.loop(PIXELS_COUNT, pixels);

  long nowAfterModelRunner = millis();
  
  Patterns::applyPixels(strip, pixels);
  strip.show();

  long nowAfterPixels = millis();

  loopLogger();

  long nowAfterEverything = millis();

  if (now - lastUpdateMS >= logDurationIntervalMS) {
    lastUpdateMS = now;
    Logger::logf("Completed loop, running model=%dms, applying LEDs=%dms, total duration %dms\n",
      nowAfterModelRunner - now, nowAfterPixels - nowAfterModelRunner, nowAfterEverything - now);
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
