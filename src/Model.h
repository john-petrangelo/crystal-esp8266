#ifndef __MODEL__
#define __MODEL__

#include <memory>

#include "lumos-arduino/lumos-arduino/defs.h"
#include "lumos-arduino/lumos-arduino/Colors.h"
#include "lumos-arduino/lumos-arduino/Logger.h"

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

/***** MAP *****/

/*
 * TODO MapModel description
 */
class MapModel : public Model {
  public:
    MapModel(char const *name, float inRangeMin, float inRangeMax, float outRangeMin, float outRangeMax,
        std::shared_ptr<Model> model)
      : Model(name), inRangeMin(inRangeMin), inRangeMax(inRangeMax),
        outRangeMin(outRangeMin), outRangeMax(outRangeMax), model(model) { }
    Color apply(float pos, float timeStamp);

    void setInRange(float inRangeMin, float inRangeMax) {
      this->inRangeMin = inRangeMin;
      this->inRangeMax = inRangeMax;
    }

  private:
    std::shared_ptr<Model> model;
    float inRangeMin, inRangeMax, outRangeMin, outRangeMax;
};

/***** REVERSE *****/

class ReverseModel : public Model {
  public:
    ReverseModel(std::shared_ptr<Model> model) : Model("ReverseModel"), model(model) { }
    Color apply(float pos, float timeStamp) { return model->apply(1.0 - pos, timeStamp); }

  private:
    std::shared_ptr<Model> model;
};

/***** TRIANGLE *****/

class Triangle : public Model {
  public: Triangle(char const *name, float rangeMin, float rangeMax, Color color) 
    : Model(name), rangeMin(rangeMin), rangeMax(rangeMax), color(color) { }
  Color apply(float pos, float timeStamp);

  private:
    float const rangeMin;
    float const rangeMax;
    Color const color;
};

/***** IDEAS *****/

/*
 * Dim
 * Adjust the brightness of the color down by the provided percent
 * 
 * Constructors:
 *   Dim(dimPercent, model) - dims all colors of the underlying model by dimPercent, expressed as 0.0-1.0
 *
 * Requires input model
 * Position and time independent
 */

/*
 * Brighten
 * Increases the brightness of the color up by the provided percent, no R, G, or B will exceed 255
 * 
 * Constructors:
 *   Brighten(brightenPercent, model) - brightens all colors of the underlying model by brightenPercent, expressed as 0.0-1.0
 * 
 * Requires input model
 * Position and time independent
 */

/*
 * Firefly
 * A firely (small light band? needs definition) flits around in a specified range with specified speed parameters (TBD)
 */

/*
 * Matrix
 * Green spots flow from one end of the strip to the other.
 * Can experiment with varying rates, sizes, brightnesses, hues.
 * 
 * Position and time dependend.
 */

/*
 * Blend
 * Blend two models together. Details TBD, but options include LERP, add, etc.
 * 
 * Requires two input models.
 * Position and time independent.
 */

/*
 * Blur
 * Performs some sort of convolution around a position to blur the colors.
 * 
 * Requires input model.
 * Position and time independent.
 */

/*
 * Lava lamp
 * Simulate a lava lamp.
 * 
 * Direction - up/down
 * Color
 */

/*
 * Warp core
 * Simulate a Star Trek warp core.
 * 
 * Direction - up/down?
 * Color?
 * Speed?
 */

/*
 * Jacob's ladder
 * Simulate the rising electrical arc of a Jacob's ladder.
 * 
 * Color?
 * Speed?
 */


#endif // __MODEL__
