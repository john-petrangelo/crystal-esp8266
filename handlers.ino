#include <memory>

#include <ArduinoJson.h>
#include <ESP8266WebServer.h>
#include <FS.h>

#include "index.h"
#include "src/lumos-arduino/lumos-arduino/Colors.h"
#include "src/lumos-arduino/lumos-arduino/Patterns.h"

#include "src/Animations.h"
#include "src/Combinations.h"
#include "src/Demos.h"
#include "src/ModelRunner.h"

void handleRoot() {
    long startMS = millis();
    File file = SPIFFS.open("/index.html", "r");
    size_t sent = server.streamFile(file, "text/html");
    file.close();
    Logger::logf("handleRoot sentBytes=%d duration=%dms\n", sent, millis() - startMS);
}

void handleStatus() {
  StaticJsonDocument<200> doc;
  doc["time"] = millis() / 1000.0;
  doc["freeHeapBytes"] = ESP.getFreeHeap();
  doc["hostname"] = hostname + ".local";
  doc["wifiMACAddress"] = WiFi.macAddress();
  doc["ipAddress"] = WiFi.localIP().toString();
  String output;
  serializeJson(doc, output);
  server.send(200, "text/json", output);
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
  modelRunner.setModel(makeCrystalPower());
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
  long now = millis();
  if(!server.hasArg("color")) {
    server.send(400, "text/plain", "Color parameter missing\n");
    return;
  }

  String colorStr = server.arg("color");
  Color color = strtol(colorStr.c_str(), 0, 16);
  server.send(200, "text/plain", "");

  modelRunner.setModel(std::make_shared<SolidModel>("net solid model", color));
  // Logger::logf("handleSolid colorStr=%s color=0x%0X duration=%dms\n",
  //   colorStr.c_str(), color, millis() - now);
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
