#include <math.h>
#include "src/lumos-arduino/lumos-arduino/Colors.h"
#include "src/lumos-arduino/lumos-arduino/Logger.h"

class Model {
  public:
    Model(char const *name) : name(name) { }

    // Returns the color that should be displayed at the specified pos at the specified time,
    // given the provided predecessor model.
    //
    // pos          the position along the strip for which we want to determine the color, given in range 0.0-1.0
    // timeStamp    time since the beginning of the run
    virtual Color apply(float pos, float timeStamp) = 0;

    char const *name;
};

/***** SOLID *****/

/*
 * Set a solid color pattern.
 */
class SolidModel : public Model {
  public:
    SolidModel(char const *name, Color color) : Model(name), color(color) {}
    Color apply(float pos, float timeStamp);

  private:
    Color color;
};

Color SolidModel::apply(float pos, float timeStamp) {
  return color;
}

/***** GRADIENT *****/

/*
 * Set a gradient color pattern. The number of defined color points is variable.
 */
class GradientModel : public Model {
  public:
    GradientModel(char const *name, int count, ...);
    Color apply(float pos, float timeStamp);

  private:
    static int const MAX_COLORS = 10;
    int count;
    Color colors[MAX_COLORS];
};

GradientModel::GradientModel(char const *name, int count, ...) : Model(name), count(count) {
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

  float colorPos = pos * (count - 1);

  // Get the two colors flanking the mapped position
  int lower = floor(colorPos);
  int upper = ceil(colorPos);

  // Linearly interpolate from the lower color to the upper color. If same, quick return.
  if (upper == lower) {
    return colors[lower];
  }

  float ratio = (colorPos - lower) / (upper- lower);
  Color color = Colors::blend(colors[lower], colors[upper], 100 * ratio);
//  Logger::logf("Gradient::apply:%s pos=%f lower=%d upper=%d color=%06X\n", name, pos, lower, upper, color);
  return color;
}

/***** ROTATE *****/

// An action that rotates or shifts lights to the left or right.
// The speed is specified in revs per second, which is the same
// how long it takes to move from position 0.0 to 1.0.
// Rotate wraps around so that once a color reaches 1.0, then
// continues at 0.0 again.
class RotateModel : public Model {
  public:
    enum Direction {
      UP, DOWN
    };
  
    RotateModel(char const *name, float revsPerSecond, Direction dir, Model *model) 
      : Model(name), revsPerSecond(revsPerSecond), dir(dir), model(model) {}
    Color apply(float pos, float timeStamp);
    ~RotateModel() { delete model; model = NULL; }

  private:
    float revsPerSecond;
    Direction dir;
    Model *model;
};

Color RotateModel::apply(float pos, float timeStamp) {
  // If there's no predecessor, then there's nothing to rotate. Bail out.
  if (model == NULL) {
    Logger::logf("RotateModel::apply:%s called when there is no model pos=%f\n", name, pos);
    return RED;
  }

  // "Rotate" really means look at a different position dependent on the time and rate of rotation.
  // First, figure out the offset to add to the position.
  float delta = timeStamp * revsPerSecond;
  if (dir == UP) {
    delta = -delta;
  }

  // Next, add the offset to the position, then correct for wrap-around
  float rotatedPos = pos + delta;
  while (rotatedPos > 1.0) {
    rotatedPos -= 1.0;
  }
  while (rotatedPos < 0.0) {
    rotatedPos += 1.0;
  }

//  Logger::logf("RotateModel::apply:%s rotating model pos=%f delta=% f rotatedPos=%f\n", name, pos, delta, rotatedPos);

  return model->apply(rotatedPos, timeStamp);
}


/***** WINDOW *****/

/*
 * TODO WindowModel description
 */
class WindowModel : public Model {
  public:
    WindowModel(char const *name, float rangeMin, float rangeMax, Model *insideRange, Model *outsideRange)
      : Model(name), rangeMin(rangeMin), rangeMax(rangeMax), insideRange(insideRange), outsideRange(outsideRange) { }
    Color apply(float pos, float timeStamp);

  private:
    Model *insideRange;
    Model *outsideRange;
    float rangeMin, rangeMax;
};

Color WindowModel::apply(float pos, float timeStamp) {
  if ((rangeMin <= pos) && (pos <= rangeMax)) {
    if (insideRange != NULL) {
//      Logger::logf("WindowModel::apply:%s pos=%f inside\n", name, pos);
      return insideRange->apply(pos, timeStamp);
    }
  } else {
    if (outsideRange != NULL) {
//      Logger::logf("WindowModel::apply:%s pos=%f outside\n", name, pos);
      return outsideRange->apply(pos, timeStamp);
    }
  }

  // If we go here then the model we're supposed to apply was NULL.
//  Logger::logf("WindowModel::apply:%s error inside=%p outside=%p\n", name, insideRange, outsideRange);
  return RED;
}

/***** MAP *****/

/*
 * TODO MapModel description
 */
class MapModel : public Model {
  public:
    MapModel(char const *name, float inRangeMin, float inRangeMax, float outRangeMin, float outRangeMax, Model *model)
      : Model(name), inRangeMin(inRangeMin), inRangeMax(inRangeMax), outRangeMin(outRangeMin), outRangeMax(outRangeMax),
        model(model) { }
    Color apply(float pos, float timeStamp);

  private:
    Model *model;
    float inRangeMin, inRangeMax, outRangeMin, outRangeMax;
};

Color MapModel::apply(float pos, float timeStamp) {
  if ((inRangeMin <= pos) && (pos <= inRangeMax)) {
    float outPos = fmap(pos, inRangeMin, inRangeMax, outRangeMin, outRangeMax);
//    Logger::logf("WindowModel::apply:%s mapping (%f,%f)->(%f,%f) pos=%f outPos=%f\n",
//      name, inRangeMin, inRangeMax, outRangeMin, outRangeMax, pos, outPos);
    return model->apply(outPos, timeStamp);
  }

  // Everything outside of the "in range" will be BLACK
//  Logger::logf("MapModel::apply:%s outside range pos=%f\n", name, pos);
  return BLACK;
}

/***** FLAME *****/

//class PurpleFlame : public Model {
//  private:
//    Color const C1 = Colors::blend(RED, BLUE, 10);
//    Color const C2 = Colors::blend(RED, BLUE, 20);
//    Color const C3 = PURPLE;
//
//  public:
//    PurpleFlame(Adafruit_NeoPixel &strip, Pixels pixels) : Action(strip, 0, strip.numPixels()), pixels(pixels) { }
//    void update();
//};
//
//void PurpleFlame::apply(float pos, float timeStamp) {
//  int const delayMS = 110;
//
//  // Paint the background all black.
//  Patterns::setSolidColor(pixels, firstPixel, lastPixel, BLACK);
//  Patterns::applyPixels(strip, pixels, firstPixel, lastPixel);
//
//  int const range = lastPixel - firstPixel;
//  int const myFirstPixel = firstPixel + random(0, range / 5);
//  int const myLastPixel = lastPixel - random(0, range / 5);
//  Patterns::setGradient(pixels, myFirstPixel, myLastPixel, 7, BLACK, C1, C2, C3, C2, C1, BLACK);
//  Patterns::applyPixels(strip, pixels, myFirstPixel, myLastPixel);
//
//  setNextUpdateMS(millis() + delayMS);
//}
