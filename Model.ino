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

    Model() {}
    Model(Model *predecessor) : predecessor(predecessor) {}
    virtual ~Model() {
      if (predecessor != NULL) {
        Logger::logMsgLn("Model::~Model deleting predecessor");
        delete predecessor;
        predecessor = NULL;
      }
    }

   protected:
    Model *predecessor = NULL;
};

/***** SOLID *****/

/*
 * Set a solid color pattern.
 */
class SolidModel : public Model {
  public:
    SolidModel(Color color) : color(color) {}
    virtual Color apply(float pos, float timeStamp) {
      printf("SolidModel::apply(%f, %f)\n", pos, timeStamp);
      return color;
    }

  private:
    Color color;
};

/***** GRADIENT *****/

/*
 * Set a gradient color pattern. The number of defined color points is variable.
 */
class GradientModel : public Model {
  public:
    GradientModel(int count, ...);
    Color apply(float pos, float timeStamp);

  private:
    static int const MAX_COLORS = 10;
    int count;
    Color colors[MAX_COLORS];
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
//  printf("GradientModel::apply(%f, %f)\n", pos, timeStamp);

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
  
    RotateModel(Model *predecessor, float revsPerSecond, Direction dir) 
      : Model(predecessor), revsPerSecond(revsPerSecond), dir(dir) {}
    Color apply(float pos, float timeStamp);

  private:
    float revsPerSecond;
    Direction dir;
};

Color RotateModel::apply(float pos, float timeStamp) {
  // If there's no predecessor, then there's nothing to rotate. Bail out.
  if (predecessor == NULL) {
    Logger::logMsgLn("RotateModel::apply called when there is no predecessor\n");
    return RED;
  }
  
  float delta = timeStamp * revsPerSecond;
  if (dir == UP) {
    delta = -delta;
  }

  // "Rotate" really means look at a different position dependent on the time and rate of rotation.
  float newPos = fmod(pos + delta, 1.0);
  if (newPos < 0.0) {
    newPos += 1.0;
  }
  
//  printf("RotateModel::apply delta=%f newPos=%f\n", delta, newPos);

  return predecessor->apply(newPos, timeStamp);
}
