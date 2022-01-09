#include <math.h>

#include "Model.h"
#include "utils.h"

/***** MULTIGRADIENT *****/

MultiGradientModel::MultiGradientModel(char const *name, int count, ...) : Model(name), count(count) {
  if (count < 2 || count > MAX_COLORS) {
    count = 0;
    return;
  }
  
  // Declare a va_list macro and initialize it with va_start.
  // Copy all of the colors from varargs to array.
  va_list argList;
  va_start(argList, count);
  for (int i = 0; i < count; i++) {
    colors[i] = va_arg(argList, Color);
  }
  va_end(argList);
}

Color MultiGradientModel::apply(float pos, float timeStamp) {
  float colorPos = pos * (count - 1);

  // Get the two colors flanking the mapped position
  int lower = floor(colorPos);
  int upper = ceil(colorPos);

  // Linearly interpolate from the lower color to the upper color. If same, quick return.
  if (upper == lower) {
    return colors[lower];
  }

  float ratio = (colorPos - lower) / (upper - lower);
  return Colors::blend(colors[lower], colors[upper], 100 * ratio);
}

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

/***** WINDOW *****/

WindowModel::~WindowModel() {
  delete insideModel;
  insideModel = NULL;
  delete outsideModel;
  outsideModel = NULL;
}

Color WindowModel::apply(float pos, float timeStamp) {
  if ((rangeMin <= pos) && (pos <= rangeMax)) {
    return insideModel->apply(pos, timeStamp);
  }

  // The pos is outside the range
  return outsideModel->apply(pos, timeStamp);
}

/***** MAP *****/

Color MapModel::apply(float pos, float timeStamp) {
  if ((inRangeMin <= pos) && (pos <= inRangeMax)) {
    float outPos = fmap(pos, inRangeMin, inRangeMax, outRangeMin, outRangeMax);
    return model->apply(outPos, timeStamp);
  }

  // Everything outside of the "in range" will be BLACK
  return BLACK;
}

/***** FLAME *****/

FlameModel::FlameModel() : Model("Flame"), lastUpdateMS(-PERIOD_MS) {
  Model *mgm = new MultiGradientModel("flame multigradient", 7, BLACK, C1, C2, C3, C2, C1, BLACK);
  mapModel = new MapModel("map multigradient",0.0, 1.0, 0.0, 1.0, mgm);
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
