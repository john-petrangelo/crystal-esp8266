#include <math.h>

#include "lumos-arduino/lumos-arduino/Colors.h"

#include "Animations.h"
#include "Combinations.h"
#include "utils.h"

/***** ROTATE *****/

Color RotateModel::apply(float pos, float timeStamp) {
  // If there's no predecessor, then there's nothing to rotate. Bail out.
  if (model == NULL) {
    return RED;
  }

  // "Rotate" really means look at a different position dependent on the time and rate of rotation.
  // First, figure out the offset to add to the position.
  float delta = timeStamp * revsPerSecond;
  if (dir == UP) {
    delta = -delta;
  }

  // Next, add the offset to the position, then correct for wrap-around
  float rotatedPos = fmod(pos + delta, 1.0);
  if (rotatedPos < 0.0) {
    rotatedPos += 1.0;
  }

  return model->apply(rotatedPos, timeStamp);
}

/***** FLAME *****/

FlameModel::FlameModel() : Model("Flame"), lastUpdateMS(-PERIOD_MS) {
  auto mgm = std::make_shared<MultiGradientModel>("flame multigradient", 7, BLACK, C1, C2, C3, C2, C1, BLACK);
  model = std::make_shared<MapModel>("map multigradient",0.0, 1.0, 0.0, 1.0, mgm);
}

Color FlameModel::apply(float pos, float timeStamp) {
  long now = millis();
  if ((now - lastUpdateMS) > PERIOD_MS) {
    lastUpdateMS = now;

    float const lower = frand(0, 0.2);
    float const upper = frand(0.8, 1.0);

    model->setInRange(lower, upper);
  }

  return model->apply(pos, timeStamp);
}

/***** PULSATE *****/

Color Pulsate::apply(float pos, float timeStamp) {
  timeStamp = fmod(timeStamp, periodSecs);
  float dimness = 0.0;
  if (timeStamp < brightenSecs) {
    // We're getting brighter
    dimness = fmap(timeStamp, 0.0, brightenSecs, brightest, dimmest);
  } else {
    // We're getting dimmer
    dimness = fmap(timeStamp, brightenSecs, periodSecs, dimmest, brightest);
  }

  Color oldColor = model->apply(pos, timeStamp);
  Color newColor = Colors::fade(oldColor, dimness * 100.0);
  return newColor;
}

/***** COMPOSITES *****/

std::shared_ptr<Model> makeDarkCrystal() {
  return makeCrystal(0xff00d0, 5000, 0xff00d0, 2000, 0xff00d0, 3000);
}

std::shared_ptr<Model> makeCrystal(
    Color upperColor, int upperSpeed,
    Color middleColor, int middleSpeed,
    Color lowerColor, int lowerSpeed) {

  Logger::logf("makeCrystal upper=(0x%X %d) middle=(0x%X %d) lower(0x%X %d)\n",
    upperColor, upperSpeed, middleColor, middleSpeed, lowerColor, lowerSpeed);

  auto upperTriangle = std::make_shared<Triangle>("crystal upper color", 0.6, 1.0, upperColor);
  float periodSecs = (11000 - upperSpeed) / 2000.0;
  Logger::logf("upperPeriodSecs=%f ", periodSecs);
  auto upperPulsate = std::make_shared<Pulsate>("crystal upper pulsate", 0.3, 1.0, periodSecs, periodSecs, upperTriangle);

  auto middleTriangle = std::make_shared<Triangle>("crystal middle color", 0.3, 0.7, middleColor);
  periodSecs = (11000 - middleSpeed) / 2000.0;
  Logger::logf("middlePeriodSecs=%f ", periodSecs);
  auto middlePulsate = std::make_shared<Pulsate>("crystal middle pulsate", 0.4, 1.0, periodSecs, periodSecs, middleTriangle);

  auto lowerTriangle = std::make_shared<Triangle>("crystal lower color", 0.0, 0.4, lowerColor);
  periodSecs = (11000 - lowerSpeed) / 2000.0;
  Logger::logf("lowerPeriodSecs=%f\n", periodSecs);
  auto lowerPulsate = std::make_shared<Pulsate>("crystal lower pulsate", 0.3, 1.0, periodSecs, periodSecs, lowerTriangle);

  auto sum = std::make_shared<Add>("sum", upperPulsate, middlePulsate);
  sum = std::make_shared<Add>("sum", sum, lowerPulsate);

  return sum;
}
