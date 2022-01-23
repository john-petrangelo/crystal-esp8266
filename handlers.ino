#include <memory>

#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <FS.h>

#include "src/lumos-arduino/lumos-arduino/Colors.h"
#include "src/lumos-arduino/lumos-arduino/Patterns.h"

#include "src/Animations.h"
#include "src/Combinations.h"
#include "src/Demos.h"
#include "src/ModelRunner.h"

void handleRoot() {
    long startMS = millis();
    File file = SPIFFS.open("/index.html", "r");
    long openedMS = millis();
    size_t sent = server.streamFile(file, "text/html");
    long streamedMS = millis();
    file.close();
    long closedMS = millis();
    Logger::logf("handleRoot sentBytes=%d open=%dms stream=%dms close=%dms total=%dms\n",
      sent, openedMS - startMS, streamedMS - openedMS, closedMS - streamedMS, closedMS - startMS);
}

void handleCSS() {
    long startMS = millis();
    File file = SPIFFS.open("/crystal.css", "r");
    long openedMS = millis();
    size_t sent = server.streamFile(file, "text/css");
    long streamedMS = millis();
    file.close();
    long closedMS = millis();
    Logger::logf("handleCSS sentBytes=%d open=%dms stream=%dms close=%dms total=%dms\n",
      sent, openedMS - startMS, streamedMS - openedMS, closedMS - streamedMS, closedMS - startMS);
}

void handleJS() {
    long startMS = millis();
    File file = SPIFFS.open("/crystal.js", "r");
    long openedMS = millis();
    size_t sent = server.streamFile(file, "text/javascript");
    long streamedMS = millis();
    file.close();
    long closedMS = millis();
    Logger::logf("handleJS sentBytes=%d open=%dms stream=%dms close=%dms total=%dms\n",
      sent, openedMS - startMS, streamedMS - openedMS, closedMS - streamedMS, closedMS - startMS);
}

void handleStatus() {
  StaticJsonDocument<200> doc;
  doc["time"] = millis() / 1000.0;

  JsonObject memory = doc.createNestedObject("memory");
  memory["freeHeapBytes"] = ESP.getFreeHeap();

  JsonObject fs = doc.createNestedObject("filesystem");
  FSInfo fsInfo;
  bool gotInfo = SPIFFS.info(fsInfo);
  if (gotInfo) {
    fs["totalBytes"] = fsInfo.totalBytes;
    fs["usedBytes"] = fsInfo.usedBytes;
    fs["blockSize"] = fsInfo.blockSize;
    fs["pageSize"] = fsInfo.pageSize;
    fs["maxOpenFiles"] = fsInfo.maxOpenFiles;
    fs["maxPathLength"] = fsInfo.maxPathLength;
  }

  JsonObject network = doc.createNestedObject("network");
  network["hostname"] = hostname + ".local";
  network["wifiMACAddress"] = WiFi.macAddress();
  network["ipAddress"] = WiFi.localIP().toString();

  String output;
  serializeJsonPretty(doc, output);
  server.send(200, "text/json", output);
  Logger::logf("handleStatus %s", output.c_str());
}

void handleGetBrightness() {
  uint8_t value = strip.getBrightness();

  // Un-apply gamma correction
  uint8_t uncorrectedValue = sqrt(value * 255);

  // Create the response.
  StaticJsonDocument<200> doc;
  doc["value"] = uncorrectedValue;
  String output;
  serializeJsonPretty(doc, output);

  server.send(200, "text/json", output);
}

void handleSetBrightness() {
  if(!server.hasArg("value")) {
    server.send(400, "text/plain", "Value parameter missing\n");
    return;
  }

  String valueStr = server.arg("value");
  uint8_t value = strtol(valueStr.c_str(), 0, 10);

  // Apply gamma correction
  uint8_t correctedValue = value * value / 255;

  strip.setBrightness(correctedValue);

  server.send(200, "text/plain", "");
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: " + server.uri();
  message += "\nMethod: "+ (server.method() == HTTP_GET) ? "GET" : "POST";
  message += String("\nArguments: ") + server.args() + "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

void handleOff() {
  modelRunner.setModel(std::make_shared<SolidModel>("off", BLACK));
  server.send(200, "text/plain", "");
}

void handleDarkCrystal() {
  Color color = 0xFF00D0;
  if(server.hasArg("color")) {
    color = strtol(server.arg("color").c_str(), 0, 16);
  }
  modelRunner.setModel(makeCrystalPower(color));
  server.send(200, "text/plain", "");
}

void handleFlame() {
  modelRunner.setModel(std::make_shared<FlameModel>());
  server.send(200, "text/plain", "");
}

void handleRainbow() {
  auto gm = std::make_shared<MultiGradientModel>("rainbow", 8, RED, VIOLET, INDIGO, BLUE, GREEN, YELLOW, ORANGE, RED);
  auto rm = std::make_shared<RotateModel>("rainbow rotate", 0.3, RotateModel::UP, gm);
  modelRunner.setModel(rm);

  server.send(200, "text/plain", "");
}

void handleSolid() {
  if(!server.hasArg("color")) {
    server.send(400, "text/plain", "Color parameter missing\n");
    return;
  }

  String colorStr = server.arg("color");
  Color color = strtol(colorStr.c_str(), 0, 16);
  modelRunner.setModel(std::make_shared<SolidModel>("net solid model", color));

  server.send(200, "text/plain", "");
}


void handleDemo1() {
  modelRunner.setModel(makeDemo1());
  server.send(200, "text/plain", "");
}

void handleDemo2() {
  modelRunner.setModel(makeDemo2());
  server.send(200, "text/plain", "");
}

void handleDemo3() {
  modelRunner.setModel(makeDemo3());
  server.send(200, "text/plain", "");
}
