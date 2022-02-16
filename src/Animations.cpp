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
  float delta = -timeStamp * revsPerSecond;

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
  return makeCrystal(0xff00d0, 5.0, 0xff00d0, 8.0, 0xff00d0, 7.0);
}

std::shared_ptr<Model> makeCrystal(
    Color upperColor, float upperPeriodSec,
    Color middleColor, float middlePeriodSec,
    Color lowerColor, float lowerPeriodSec) {

  auto upperTriangle = std::make_shared<Triangle>("crystal upper color", 0.6, 1.0, upperColor);
  std::shared_ptr<Model> upperPulsate = upperTriangle;
  if (upperPeriodSec <= 10.0) {
    upperPulsate = std::make_shared<Pulsate>("crystal upper pulsate", 0.3, 1.0, upperPeriodSec/2.0, upperPeriodSec/2.0, upperTriangle);
  }

  auto middleTriangle = std::make_shared<Triangle>("crystal middle color", 0.3, 0.7, middleColor);
  std::shared_ptr<Model> middlePulsate = middleTriangle;
  if (middlePeriodSec <= 10.0) {
    middlePulsate = std::make_shared<Pulsate>("crystal middle pulsate", 0.4, 1.0, middlePeriodSec/2.0, middlePeriodSec/2.0, middleTriangle);
  }

  auto lowerTriangle = std::make_shared<Triangle>("crystal lower color", 0.0, 0.4, lowerColor);
  std::shared_ptr<Model> lowerPulsate = lowerTriangle;
  if (lowerPeriodSec <= 10.0) {
    lowerPulsate = std::make_shared<Pulsate>("crystal lower pulsate", 0.3, 1.0, lowerPeriodSec/2.0, lowerPeriodSec/2.0, lowerTriangle);
  }

  auto sum = std::make_shared<Add>("sum", upperPulsate, middlePulsate);
  sum = std::make_shared<Add>("sum", sum, lowerPulsate);

  return sum;
}
