#ifndef __MODEL__
#define __MODEL__

// #include "lumos-arduino/lumos-arduino/Actions.h"
#include "lumos-arduino/lumos-arduino/defs.h"
#include "lumos-arduino/lumos-arduino/Colors.h"
#include "lumos-arduino/lumos-arduino/Logger.h"
// #include "lumos-arduino/lumos-arduino/Patterns.h"

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

/***** REVERSE *****/

class ReverseModel : public Model {
  public:
    ReverseModel(Model *model) : Model("ReverseModel"), model(model) { }
    ~ReverseModel() { delete model; model = NULL; }
    Color apply(float pos, float timeStamp) { return model->apply(1.0 - pos, timeStamp); }

  private:
    Model *model;
};

/***** IDEAS *****/

/*
 * Pulsate
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
