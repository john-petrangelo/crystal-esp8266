#import "index.h"
#include "src/lumos-arduino/lumos-arduino/Colors.h"
#include "src/lumos-arduino/lumos-arduino/Patterns.h"

void redirectHome() {
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void handleRoot() {
  String s = MAIN_page;
  server.send(200, "text/html", s);
}

void handleStatus() {
//  String message = lumos.describe();
  String message = "No lumos\n";
  message += "NETWORK\n";
  message += "hostname: " + hostname + ".local\n";
  message += "WiFi MAC Address: " + WiFi.macAddress() + "\n";
  server.send(200, "text/plain", message);
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
  modelRunner.setModel(new SolidModel("off", BLACK));
  redirectHome();
}

void handleDarkCrystal() {
//  lumos.runForever(&crystalLight);

  redirectHome();
}

void handleFlame() {
  modelRunner.setModel(new FlameModel);
  redirectHome();
}

void handleRainbow() {
  Model *gm = new MultiGradientModel("rainbow", 8, RED, VIOLET, INDIGO, BLUE, GREEN, YELLOW, ORANGE, RED);
  Model *rm = new RotateModel("rainbow rotate", 0.3, RotateModel::UP, gm);
  modelRunner.setModel(rm);

  redirectHome();
}
