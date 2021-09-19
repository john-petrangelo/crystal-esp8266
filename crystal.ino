#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <Adafruit_NeoPixel.h>
#include "src/lumos-arduino/lumos-arduino/Effect.h"
#include "src/lumos-arduino/lumos-arduino/Patterns.h"
#include "src/lumos-arduino/lumos-arduino/Runner.h"

// Secrets are defined in another file called "secrets.h" to avoid commiting secrets
// into a public repo. You will need to change the secret values in secrets.h to
// connect your device to your network.
#include "secrets.h"

// Configuration information about the NeoPixel strip we are using.
//int8_t const NUM_PIXELS = 16;
int8_t const PIXELS_PIN = 2;

Adafruit_NeoPixel strip(NUM_PIXELS, PIXELS_PIN, NEO_GRB + NEO_KHZ800);
ESP8266WebServer server(80);

PixelsArray pixels1;
PixelsArray pixels2;
PixelsArray pixels3;
Rotate rotate(8, RIGHT);
Flame flame(pixels3);
Runner runner = Runner::runForever(&flame);

void setup(void) {
  setupPixels();
  setupSerial();
  setupWiFi();
  setupHTTP();
  setupMDNS();
  setupOTA();
}

void loop(void) {
  server.handleClient();
  MDNS.update();
  ArduinoOTA.handle();

  runner.loop();
}

// Setup the NeoPixel LED strip
void setupPixels() {
  strip.begin();

  Patterns::setSolidColor(pixels1, 0, strip.numPixels(), BLACK);
  Patterns::applyPixels(pixels1);
  strip.show();

  strip.setBrightness(255);
}

// Setup serial communications channel
void setupSerial() {
  Serial.begin(115200);
  Serial.println("");
}

// Get the device setup on the local network
void setupWiFi() {
    // Setup WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(SECRET_SSID, SECRET_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");

  Serial.print("Network: ");
  Serial.println(SECRET_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

// Setup the web server and handlers
void setupHTTP() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/status", HTTP_GET, handleStatus);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

// Setup an MDNS responder so we can be found by <host>.local instead of IP address
void setupMDNS() {
  if (MDNS.begin("crystal")) {
    Serial.println("MDNS responder started: crystal.local");
  }
}

// Setup OTA updates
void setupOTA() {
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("crystal");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("OTA Start");

    Patterns::setSolidColor(pixels1, BLACK);
    Patterns::setSolidColor(pixels1, strip.numPixels()-1, strip.numPixels(), WHITE);
    Patterns::applyPixels(pixels1);
    strip.show();
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));

    int otaPixels = progress / (total / (strip.numPixels()-1));
    Serial.printf("progress: %u  total: %u  otaPixels: %u\r", progress, total, otaPixels);
    Patterns::setSolidColor(pixels1, 0, otaPixels, GREEN);
    Patterns::applyPixels(pixels1);
    strip.show();
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA End");

    Patterns::setSolidColor(pixels1, BLACK);
    Patterns::applyPixels(pixels1);
    strip.show();
});
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("OTA Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("OTA Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("OTA Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("OTA Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("OTA End Failed");

    Patterns::setSolidColor(pixels1, RED);
    Patterns::applyPixels(pixels1);
    strip.show();
  });
  
  ArduinoOTA.begin();
  Serial.println("OTA ready");  
}
