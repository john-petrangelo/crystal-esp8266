#include <math.h>
#include "src/lumos-arduino/lumos-arduino/Colors.h"
#include <stdio.h>

class Model {
  public:
    // Returns the color that should be displayed at the specified pos at the specified time,
    // given the provided predecessor model.
    //
    // pos          the position along the strip for which we want to determine the color, given in range 0.0-1.0
    // timeStamp    time since the beginning of the run
    virtual Color apply(float pos, float timeStamp) = 0;
};

/*
 * Set a gradient color pattern. The number of defined color points is variable.
 * 
 * Example: to run a gradient from red to yellow to blue:
 *     setGradient(pixels, 0, strip.numPixels(), 3, RED, YELLOW, BLUE);
 */
class GradientModel : public Model {
  private:
    static int const MAX_COLORS = 10;
    int count;
    Color colors[MAX_COLORS];

  public:
    GradientModel(int count, ...);
    Color apply(float pos, float timeStamp);
};

GradientModel::GradientModel(int count, ...) : count(count) {
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

Color GradientModel::apply(float pos, float timeStamp) {
  // GradientModel is static and ignores timeStamp

  // Map the position from [0.0, 1.0] to [0, count-1]
  float colorPos = pos * (count - 1);

  // Get the two colors flanking the mapped position
  int lower = floor(colorPos);
  int upper = ceil(colorPos);

  // Linearly interpolate from the lower color to the upper color. If same, quick return.
  if (upper == lower) {
    return colors[lower];
  }

  float ratio = (colorPos - lower) / (upper- lower);
  return Colors::blend(colors[lower], colors[upper], 100 * ratio);
}
