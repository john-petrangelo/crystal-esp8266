#include <math.h>

#include "Animations.h"
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
  mapModel = std::make_shared<MapModel>("map multigradient",0.0, 1.0, 0.0, 1.0, mgm);
}

Color FlameModel::apply(float pos, float timeStamp) {
  long now = millis();
  if ((now - lastUpdateMS) > PERIOD_MS) {
    lastUpdateMS = now;

    float const lower = frand(0, 0.2);
    float const upper = frand(0.8, 1.0);

    mapModel->setInRange(lower, upper);
  }

  return mapModel->apply(pos, timeStamp);
}
