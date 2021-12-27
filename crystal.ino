#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ArduinoOTA.h>
#include <Adafruit_NeoPixel.h>

#include "src/lumos-arduino/lumos-arduino/Actions.h"
#include "src/lumos-arduino/lumos-arduino/Logger.h"
#include "src/lumos-arduino/lumos-arduino/Lumos.h"
#include "src/lumos-arduino/lumos-arduino/Patterns.h"

// Secrets are defined in another file called "secrets.h" to avoid commiting secrets
// into a public repo. You will need to change the secret values in secrets.h to
// connect your device to your network.
#include "secrets.h"

// Configuration information about the NeoPixel strip we are using.
int16_t const PIXELS_COUNT = 24;
int8_t const PIXELS_PIN = 2;

String hostname = "crystal";
ESP8266WebServer server(80);

// Server used for logging.
WiFiServer logServer(8000);
WiFiClient logClient;

Lumos lumos(PIXELS_COUNT, PIXELS_PIN);
PixelsArray pixels;
CrystalLight crystalLight(lumos.getStrip(), pixels);
Flame flame(lumos.getStrip(), pixels);
Rotate rotate(lumos.getStrip(), 5, RIGHT);

void setup(void) {
  setupSerial();
  setupWiFi();
  setupHTTP();
  setupMDNS();
  setupOTA();

  logServer.begin();

  lumos.runForever(&crystalLight);
}

int count = 0;

void loop(void) {
  // Check for network activity.
  server.handleClient();
  MDNS.update();
  ArduinoOTA.handle();

  // Animate the LEDs. 
  lumos.loop();

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

  // If we recognize the MAC address, use a different hostname specific to that MAC address
  String macAddress = WiFi.macAddress();
  if (macAddress == "E8:DB:84:98:7F:C3") {
    hostname = "shard";
  } else if (macAddress = "84:CC:A8:81:0A:53") {
    hostname = "crystal";
  } else {
    hostname = WiFi.hostname();
  }
  Serial.print("Network: ");
  Serial.println(SECRET_SSID);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.print("MAC address: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Hostname: ");
  Serial.println(hostname);
}

// Setup the web server and handlers
void setupHTTP() {
  server.on("/", HTTP_GET, handleRoot);
  server.on("/status", HTTP_GET, handleStatus);
  server.on("/off", HTTP_GET, handleOff);
  server.on("/darkcrystal", HTTP_GET, handleDarkCrystal);
  server.on("/flame", HTTP_GET, handleFlame);
  server.on("/rainbow", HTTP_GET, handleRainbow);
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}

// Setup an MDNS responder so we can be found by <host>.local instead of IP address
void setupMDNS() {
  if (MDNS.begin("hostname")) {
    Serial.println("MDNS responder started: crystal.local");
  }
}

// Setup OTA updates
void setupOTA() {
  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(hostname.c_str());

  // No authentication by default
  // ArduinoOTA.setPassword((const char *)"123");

  ArduinoOTA.onStart([]() {
    Serial.println("OTA Start");

    Patterns::setSolidColor(lumos.getStrip(), pixels, BLACK);
    Patterns::setSolidColor(pixels, lumos.getStrip().numPixels()-1, lumos.getStrip().numPixels(), WHITE);
    Patterns::applyPixels(lumos.getStrip(), pixels);
    lumos.getStrip().show();
  });
  
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("OTA Progress: %u%%\r", (progress / (total / 100)));

    int otaPixels = progress / (total / lumos.getStrip().numPixels());
    Serial.printf("progress: %u  total: %u  otaPixels: %u\r", progress, total, otaPixels);
    Patterns::setSolidColor(pixels, 0, otaPixels, GREEN);
    Patterns::applyPixels(lumos.getStrip(), pixels);
    lumos.getStrip().show();
  });
  
  ArduinoOTA.onEnd([]() {
    Serial.println("\nOTA End");

    Patterns::setSolidColor(lumos.getStrip(), pixels, BLACK);
    Patterns::applyPixels(lumos.getStrip(), pixels);
    lumos.getStrip().show();
});
  
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("OTA Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("OTA Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("OTA Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("OTA Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("OTA Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("OTA End Failed");

    Patterns::setSolidColor(lumos.getStrip(), pixels, RED);
    Patterns::applyPixels(lumos.getStrip(), pixels);
    lumos.getStrip().show();
  });
  
  ArduinoOTA.begin();
  Serial.println("OTA ready");
}
