#include <math.h>

#include "lumos-arduino/lumos-arduino/Colors.h"
#include "lumos-arduino/lumos-arduino/Logger.h"

#include "Combinations.h"
#include "utils.h"

/***** ADD *****/

Color Add::apply(float pos) {
  Color const colorA = modelA->apply(pos);
  Color const colorB = modelB->apply(pos);

  Color const newColor = Colors::add(colorA, colorB);
  return newColor;
}

/***** WINDOW *****/

Color WindowModel::apply(float pos) {
  if ((rangeMin <= pos) && (pos <= rangeMax)) {
    return insideModel->apply(pos);
  }

  // The pos is outside the range
  return outsideModel->apply(pos);
}
