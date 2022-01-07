#include <math.h>
#include "src/lumos-arduino/lumos-arduino/Actions.h"
#include "src/lumos-arduino/lumos-arduino/Colors.h"
#include "src/lumos-arduino/lumos-arduino/Logger.h"
#include "src/lumos-arduino/lumos-arduino/Patterns.h"

class Model {
  public:
    Model(char const *name) : name(name) { }
    virtual ~Model() { }

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
 * Set a gradient color pattern from one color to another color.
 */
class GradientModel : public Model {
  public:
    GradientModel(char const *name, Color a, Color b) : Model(name), a(a), b(b) { }
    Color apply(float pos, float timeStamp);

  private:
    Color a, b;
};

Color GradientModel::apply(float pos, float timeStamp) {
  // Linearly interpolate from the lower color to the upper color.
  return Colors::blend(a, b, 100 * pos);
}

/***** MULTIGRADIENT *****/

/*
 * Set a gradient color pattern. The number of defined color points is variable.
 */
class MultiGradientModel : public Model {
  public:
    MultiGradientModel(char const *name, int count, ...);
    Color apply(float pos, float timeStamp);

  private:
    static int const MAX_COLORS = 10;
    int count;
    Color colors[MAX_COLORS];
};

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
//  Logger::logf("Gradient::apply:%s pos=%f\n", name, pos);
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

/*
 * TODO WindowModel description
 */
class WindowModel : public Model {
  public:
    WindowModel(char const *name, float rangeMin, float rangeMax, Model *insideModel, Model *outsideModel)
      : Model(name), rangeMin(rangeMin), rangeMax(rangeMax), insideModel(insideModel), outsideModel(outsideModel) { }
    ~WindowModel();
    Color apply(float pos, float timeStamp);

  private:
    Model *insideModel;
    Model *outsideModel;
    float rangeMin, rangeMax;
};

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

/*
 * TODO MapModel description
 */
class MapModel : public Model {
  public:
    MapModel(char const *name, float inRangeMin, float inRangeMax, float outRangeMin, float outRangeMax, Model *model)
      : Model(name), inRangeMin(inRangeMin), inRangeMax(inRangeMax), outRangeMin(outRangeMin), outRangeMax(outRangeMax),
        model(model) { }
    ~MapModel() { delete model; model = NULL; }
    Color apply(float pos, float timeStamp);

    void setInRange(float inRangeMin, float inRangeMax) {
      this->inRangeMin = inRangeMin;
      this->inRangeMax = inRangeMax;
    }

  private:
    Model *model;
    float inRangeMin, inRangeMax, outRangeMin, outRangeMax;
};

Color MapModel::apply(float pos, float timeStamp) {
  if ((inRangeMin <= pos) && (pos <= inRangeMax)) {
    float outPos = fmap(pos, inRangeMin, inRangeMax, outRangeMin, outRangeMax);
    return model->apply(outPos, timeStamp);
  }

  // Everything outside of the "in range" will be BLACK
  return BLACK;
}

/***** FLAME *****/

class FlameModel : public Model {
  public:
    FlameModel();
    ~FlameModel() { delete mapModel, mapModel = NULL; }

    Color apply(float pos, float timeStamp);

  private:
    Color const C1 = Colors::blend(RED, YELLOW, 10);
    Color const C2 = Colors::blend(RED, YELLOW, 20);
    Color const C3 = ORANGE;

    MapModel *mapModel;

    long lastUpdateMS;
    int const PERIOD_MS = 110;
};

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

/***** REVERSE *****/

class ReverseModel : public Model {
  public:
    ReverseModel(char const *name) : Model(name) { }
    ~ReverseModel() { delete model; model = NULL; }
    Color apply(float pos, float timeStamp);

  private:
    Model *model;
};

Color ReverseModel::apply(float pos, float timeStamp) {
  return model->apply(1.0 - pos, timeStamp);
}
