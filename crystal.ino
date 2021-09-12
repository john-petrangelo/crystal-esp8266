#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <Adafruit_NeoPixel.h>

// Secrets are defined in another file called "secrets.h" to avoid commiting secrets
// into a public repo. You will need to change the secret values in secrets.h to
// connect your device to your network.
#include "secrets.h"

// Configuration information about the NeoPixel strip we are using.
int8_t const NUM_PIXELS = 16;
int8_t const PIXELS_PIN = 2;

Adafruit_NeoPixel strip(NUM_PIXELS, PIXELS_PIN, NEO_GRB + NEO_KHZ800);
ESP8266WebServer server(80);

void setup(void) {
  setupSerial();
  setupPixels();
  setupWiFi();
  setupHTTP();
  setupMDNS();
  setupOTA();
}

void loop(void) {
  server.handleClient();
  MDNS.update();
  ArduinoOTA.handle();
  int const brightness = 255;
  colorWipe(strip.Color(brightness, 0, 0), 75);
  colorWipe(strip.Color(0, brightness, 0), 75);
  colorWipe(strip.Color(0, 0, brightness), 75);
  colorWipe(strip.Color(0, 0, 0), 75);
}

// Setup serial communications channel
void setupSerial() {
  Serial.begin(115200);
  Serial.println("");
}

// Setup the NeoPixel LED strip
void setupPixels() {
  strip.begin();
  strip.show();
  strip.setBrightness(20);
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
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

// Setup an MDNS responder so we can be found by <host>.local instead of IP address
void setupMDNS() {
  if (MDNS.begin(SECRET_HOST)) {
    Serial.print("MDNS responder started: ");
    Serial.print(SECRET_HOST);
    Serial.println(".local");
  }
}

// Setup OTA updates
void setupOTA() {
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("esp8266");

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("OTA Start");
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA End");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("OTA Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("OTA Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("OTA Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("OTA Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("OTA End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA ready");  
}

void colorWipe(uint32_t color, int wait) {
  for(int i=0; i<strip.numPixels(); i++) { // For each pixel in strip...
    strip.setPixelColor(i, color);         //  Set pixel's color (in RAM)
    strip.show();                          //  Update strip to match
    delay(wait);                           //  Pause for a moment
  }
}
