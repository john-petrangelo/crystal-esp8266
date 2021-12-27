#include "src/lumos-arduino/lumos-arduino/Actions.h"
#include "src/lumos-arduino/lumos-arduino/Colors.h"

class CrystalLight : public Action {
  private:
    Pixels const pixels;

    Color const C1 = Colors::blend(RED, BLUE, 10);
    Color const C2 = Colors::blend(RED, BLUE, 20);
    Color const C3 = PURPLE;

  public:
    CrystalLight(Adafruit_NeoPixel &strip, Pixels pixels) : Action(strip, 0, strip.numPixels()), pixels(pixels) { }
    void reset() { name = "CrystalLight"; }
    void update();
};

void CrystalLight::update() {
  int const delayMS = 110;

  // Paint the background all black.
  Patterns::setSolidColor(pixels, firstPixel, lastPixel, BLACK);
  Patterns::applyPixels(strip, pixels, firstPixel, lastPixel);

  int const range = lastPixel - firstPixel;
  int const myFirstPixel = firstPixel + random(0, range / 5);
  int const myLastPixel = lastPixel - random(0, range / 5);
  Patterns::setGradient(pixels, myFirstPixel, myLastPixel, 7, BLACK, C1, C2, C3, C2, C1, BLACK);
  Patterns::applyPixels(strip, pixels, myFirstPixel, myLastPixel);

  setNextUpdateMS(millis() + delayMS);
}
