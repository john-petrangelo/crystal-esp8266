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
