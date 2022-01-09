#ifndef __MODEL__
#define __MODEL__

// #include <math.h>
#include "lumos-arduino/lumos-arduino/Actions.h"
#include "lumos-arduino/lumos-arduino/Colors.h"
#include "lumos-arduino/lumos-arduino/Logger.h"
#include "lumos-arduino/lumos-arduino/Patterns.h"

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
    Color apply(float pos, float timeStamp) { return color; }

  private:
    Color color;
};

/***** GRADIENT *****/

/*
 * Set a gradient color pattern from one color to another color.
 */
class GradientModel : public Model {
  public:
    GradientModel(char const *name, Color a, Color b) : Model(name), a(a), b(b) { }
    Color apply(float pos, float timeStamp) { return Colors::blend(a, b, 100 * pos); }

  private:
    Color a, b;
};

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

/***** REVERSE *****/

class ReverseModel : public Model {
  public:
    ReverseModel(Model *model) : Model("ReverseModel"), model(model) { }
    ~ReverseModel() { delete model; model = NULL; }
    Color apply(float pos, float timeStamp) { return model->apply(1.0 - pos, timeStamp); }

  private:
    Model *model;
};

/***** DEMOS *****/

class DemoModel : public Model {
  public:
    DemoModel(char const *name) : Model(name) { }
    ~DemoModel() { delete model; model = NULL; }
    Color apply(float pos, float timeStamp) { model->apply(pos, timeStamp); };

  protected:
    Model *model;
};

class Demo1 : public DemoModel {
  public:
    Demo1() : DemoModel("Demo1") {
      Model *grad_left = new GradientModel("grad-left", RED, YELLOW);
      Model *grad_right = new GradientModel("grad-right", BLUE, GREEN);
    
      Model *rot_left = new RotateModel("rotate down", 4.0, RotateModel::DOWN, grad_left);
      Model *rot_right = new RotateModel("rotate up", 1.0, RotateModel::UP, grad_right);
    
      Model *map_left = new MapModel("map left", 0.0, 0.2, 0.0, 1.0, rot_left);
      Model *map_right = new MapModel("map right", 0.2, 1.0, 0.0, 1.0, rot_right);
    
      Model *window = new WindowModel("window", 0.0, 0.2, map_left, map_right);
      Model *rot_window = new RotateModel("rotate window", 0.5, RotateModel::DOWN, window);
    
      model = rot_window;
    }

    Color apply(float pos, float timeStamp) { return model->apply(pos, timeStamp); }
};

class Demo2 : public DemoModel {
  public:
    Demo2() : DemoModel("Demo2") {
      Model *gradient = new GradientModel("grad", BLUE, RED);
      Model *rot_grad = new RotateModel("Rotate Gradient", 2.0, RotateModel::UP, gradient);
      Model *rev_grad = new ReverseModel(rot_grad);

      Model *map_left = new MapModel("map left", 0.0, 0.5, 0.0, 1.0, rot_grad);
      Model *map_right = new MapModel("map right", 0.5, 1.0, 0.0, 1.0, rev_grad);

      Model *window = new WindowModel("window", 0.0, 0.5, map_left, map_right);

      model = window;
    }

    Color apply(float pos, float timeStamp) { return model->apply(pos, timeStamp); }
};

/***** IDEAS *****/

/*
 * Pulse
 * 
 * Adjust the brightness of the underlying model between two percentages over a given period.
 * For example, the brightness may vary from 20% to 100% and back down to 20% over 3 seconds.
 * 
 * Constructors:
 *   Pulse(period, model) - varies 0%-100% over the period in seconds, period is divided evenly between up and down
 *   Pulse(dimmest, brightest, model) - varies between dimmest and brightest over the period
 *   Pulse(brightest, dimmest, upTime, dimSec, brightenSecs) - varies between dimmest and brightest,
 *      taking dimSecs to dim and brightenSecs to brighten
 *    
 * Requires underlying model
 * Position independent, time dependent
 */

 /*
  * Dim
  * Adjust the brightness of the color down by the provided percent
  * 
  * Constructors:
  *   Dim(dimPercent, model) - dims all colors of the underlying model by dimPercent, expressed as 0.0-1.0
  * 
  * Requires underlying model
  * Position and time independent
  */
  
 /*
  * Brighten
  * Increases the brightness of the color up by the provided percent, no R, G, or B will exceed 255
  * 
  * Constructors:
  *   Brighten(brightenPercent, model) - brightens all colors of the underlying model by brightenPercent, expressed as 0.0-1.0
  * 
  * Requires underlying model
  * Position and time independent
  */

  /*
   * Firefly
   * A firely (small light band? needs definition) flits around in a specified range with specified speed parameters (TBD)
   */
 
#endif // __MODEL__
