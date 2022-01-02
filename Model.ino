#include <math.h>
#include "src/lumos-arduino/lumos-arduino/Colors.h"
#include "src/lumos-arduino/lumos-arduino/Logger.h"

class Model {
  public:
    // Returns the color that should be displayed at the specified pos at the specified time,
    // given the provided predecessor model.
    //
    // pos          the position along the strip for which we want to determine the color, given in range 0.0-1.0
    // timeStamp    time since the beginning of the run
    virtual Color apply(float pos, float timeStamp) = 0;

    Model(float rangeMin, float rangeMax, Model *predecessor, char const *name)
        : rangeMin(rangeMin), rangeMax(rangeMax), predecessor(predecessor), name(name) { }
    virtual ~Model();

    float getRangeMin() { return rangeMin; }
    float getRangeMax() { return rangeMax; }
    char const *getName() { return name; }
    
  protected:
    bool isInRange(float pos) {
      return (rangeMin <= pos) && (pos <= rangeMax);
    }
  
    Model *predecessor;
    float rangeMin;
    float rangeMax;
    char const *name;
};

Model::~Model() {
  if (predecessor != NULL) {
    Logger::logMsgLn("Model::~Model deleting predecessor");
    delete predecessor;
    predecessor == NULL;
  }
}

/***** SOLID *****/

/*
 * Set a solid color pattern.
 */
class SolidModel : public Model {
  public:
    SolidModel(Color color, float rangeMin = 0.0, float rangeMax = 1.0, Model *predecessor = NULL, char const *name = "")
      : Model(rangeMin, rangeMax, predecessor, name), color(color) {}
    Color apply(float pos, float timeStamp);

  private:
    Color color;
};

Color SolidModel::apply(float pos, float timeStamp) {
  if (predecessor != NULL) {
    return predecessor->apply(pos, timeStamp);
  } else {
    return BLACK;
  }

  return color;
}


/***** GRADIENT *****/

/*
 * Set a gradient color pattern. The number of defined color points is variable.
 */
class GradientModel : public Model {
  public:
    GradientModel(float rangeMin, float rangeMax, char const *name, int count, ...);
    Color apply(float pos, float timeStamp);

  private:
    static int const MAX_COLORS = 10;
    int count;
    Color colors[MAX_COLORS];
};

GradientModel::GradientModel(float rangeMin, float rangeMax, char const *name, int count, ...)
    : Model(rangeMin, rangeMax, NULL, name), count(count) {
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
  if (!isInRange(pos)) {
    if (predecessor != NULL) {
      return predecessor->apply(pos, timeStamp);
    } else {
      Logger::logf("GradientModel::apply:%3s not in range pos=%f\n", pos, name);
      return BLUE;
    }
  }

  // GradientModel is static and ignores timeStamp

  // Map the position from [rangeMin, rangeMax] to [0, count-1]
  pos = fmap(pos, rangeMin, rangeMax, 0.0, 1.0);
  float colorPos = pos * (count - 1);

  Logger::logf("GradientMode::apply:%3s mapped pos=%f\n", name, pos);

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
  
    RotateModel(float revsPerSecond, Direction dir, float rangeMin, float rangeMax, Model *predecessor, char const *name) 
      : Model(rangeMin, rangeMax, predecessor, name), revsPerSecond(revsPerSecond), dir(dir) {}
    Color apply(float pos, float timeStamp);

  private:
    float revsPerSecond;
    Direction dir;
};

Color RotateModel::apply(float pos, float timeStamp) {
  // If there's no predecessor, then there's nothing to rotate. Bail out.
  if (predecessor == NULL) {
    Logger::logf("RotateModel::apply:%3s called when there is no predecessor pos=%f\n", name, pos);
    return RED;
  }

  if (!isInRange(pos)) {
    Logger::logf("RotateModel::apply:%3s not in range pos=%f\n", name, pos);
    return predecessor->apply(pos, timeStamp);
  }

  float rangeWidth = rangeMax - rangeMin;

  // "Rotate" really means look at a different position dependent on the time and rate of rotation.
  // First, figure out the offset to add to the position.
  float delta = timeStamp * revsPerSecond * rangeWidth;
  if (dir == UP) {
    delta = -delta;
  }

  // Next, add the offset to the position, then correct for wrap-around
  float rotatedPos = pos + delta;
  while (rotatedPos > rangeMax) {
    rotatedPos -= rangeWidth;
  }
  while (rotatedPos < rangeMin) {
    rotatedPos += rangeWidth;
  }

  // Finally, apply the range mapping to the position
  rotatedPos = fmap(rotatedPos, rangeMin, rangeMax, predecessor->getRangeMin(), predecessor->getRangeMax());
  
  Logger::logf("RotateModel::apply:%3s rotating predecessor pos=%f delta=% f rotatedPos=%f\n", name, pos, delta, rotatedPos);

  return predecessor->apply(rotatedPos, timeStamp);
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
