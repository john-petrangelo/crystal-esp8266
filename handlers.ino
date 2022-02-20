#include <memory>

#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <LittleFS.h>

#include "src/lumos-arduino/lumos-arduino/Colors.h"
#include "src/lumos-arduino/lumos-arduino/Patterns.h"

#include "src/Animations.h"
#include "src/Combinations.h"
#include "src/Demos.h"
#include "src/Renderer.h"
#include "src/utils.h"

void handleRoot() {
    long startMS = millis();
    File file = LittleFS.open("/index.html", "r");
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
    File file = LittleFS.open("/crystal.css", "r");
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
    File file = LittleFS.open("/crystal.js", "r");
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
  bool gotInfo = LittleFS.info(fsInfo);
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

void handleCrystal() {
  StaticJsonDocument<384> doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  if (error) {
    Logger::logf("handleCrystal failed to parse JSON: %s\n", error.c_str());
    server.send(400, "text/plain", error.c_str());
    return;
  }

  Color upperColor = strtol(doc["upper"]["color"], 0, 16);
  Color middleColor = strtol(doc["middle"]["color"], 0, 16);
  Color lowerColor = strtol(doc["lower"]["color"], 0, 16);
  float upperSpeed = doc["upper"]["speed"];
  float middleSpeed = doc["middle"]["speed"];
  float lowerSpeed = doc["lower"]["speed"];

  float upperPeriodSec = fmap(upperSpeed, 0.0, 1.0, 11.0, 1.0);
  float middlePeriodSec = fmap(middleSpeed, 0.0, 1.0, 11.0, 1.0);
  float lowerPeriodSec = fmap(lowerSpeed, 0.0, 1.0, 11.0, 1.0);

  renderer.setModel(makeCrystal(
    upperColor, upperPeriodSec,
    middleColor, middlePeriodSec,
    lowerColor, lowerPeriodSec));
  server.send(200, "text/plain", "");
}

void handleFlame() {
  renderer.setModel(std::make_shared<Flame>());
  server.send(200, "text/plain", "");
}

void handleRainbow() {
  StaticJsonDocument<128> doc;
  DeserializationError error = deserializeJson(doc, server.arg("plain"));
  if (error) {
    Logger::logf("handleRainbow failed to parse JSON: %s\n", error.c_str());
    server.send(400, "text/plain", error.c_str());
    return;
  }

  String mode = doc["mode"];
  float speed = doc["speed"];

  std::shared_ptr<Model> gm = nullptr;
  if (mode == "vivid") {
    gm = std::make_shared<MultiGradientModel>("rainbow-gradient",
      8, RED, VIOLET, INDIGO, BLUE, GREEN, YELLOW, ORANGE, RED);
  } else if (mode == "pastel") {
    gm = std::make_shared<MultiGradientModel>("rainbow-gradient",
      5, 0x8080FF, 0x80FF80, 0xFFFF80, 0xFFD780, 0xFF8080);
  // } else if (mode == "retro") {
  // }
  } else if (mode == "tv") {
    gm = std::make_shared<MultiGradientModel>("rainbow-gradient",
      8, 0x0000BF, 0xBF0000, 0xBF00C0, 0x00BF00, 0x00BFBE, 0xBFBF00, 0xBFBFBF, 0x666666);
  // }
  // } else if (mode == "monochrome") {
  } else {
    // Assume "classic" as a default
    gm = std::make_shared<MultiGradientModel>("rainbow-gradient",
      8, RED, VIOLET, INDIGO, BLUE, GREEN, YELLOW, ORANGE, RED);
  }

  auto model = renderer.getModel();
  if (model->getName() == "rainbow-rotate") {
    auto rainbowModel = static_cast<Rotate*>(model.get());
    if (rainbowModel != NULL) {
      rainbowModel->setSpeed(speed);
      rainbowModel->setModel(gm);
      server.send(200, "text/plain", "");
      return;
    }
  }

  auto rm = std::make_shared<Rotate>("rainbow-rotate", speed, gm);
  renderer.setModel(rm);

  server.send(200, "text/plain", "");
}

void handleSolid() {
  if(!server.hasArg("color")) {
    server.send(400, "text/plain", "Color parameter missing\n");
    return;
  }

  String colorStr = server.arg("color");
  Color color = strtol(colorStr.c_str(), 0, 16);
  renderer.setModel(std::make_shared<SolidModel>("net solid model", color));

  server.send(200, "text/plain", "");
}


void handleDemo1() {
  renderer.setModel(makeDemo1());
  server.send(200, "text/plain", "");
}

void handleDemo2() {
  renderer.setModel(makeDemo2());
  server.send(200, "text/plain", "");
}

void handleDemo3() {
  renderer.setModel(makeDemo3());
  server.send(200, "text/plain", "");
}
