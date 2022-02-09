#ifndef __ANIMATIONS__
#define __ANIMATIONS__

#include <memory>

#include "lumos-arduino/lumos-arduino/Colors.h"
#include "Model.h"

class FlameModel : public Model {
  public:
    FlameModel();

    Color apply(float pos, float timeStamp);

  private:
    Color const C1 = Colors::blend(RED, YELLOW, 50);
    Color const C2 = Colors::blend(RED, YELLOW, 70);
    Color const C3 = Colors::blend(RED, YELLOW, 90);

    std::shared_ptr<MapModel> model;

    long lastUpdateMS;
    int const PERIOD_MS = 110;
};

/*
 * Pulsate
 * 
 * Adjust the brightness of the underlying model between two percentages over a given period.
 * For example, the brightness may vary from 20% to 100% and back down to 20% over 3 seconds.
 * 
 * Constructors:
 *   Pulse(dimmest, brightest, dimSecs, brightenSecs, model) - varies between dimmest and brightest,
 *      taking dimSecs to dim and brightenSecs to brighten
 *
 * Future constructors:
 *   Pulse(period, model) - varies 0%-100% over the period in seconds, period is divided evenly between up and down
 *   Pulse(dimmest, brightest, period, model) - varies between dimmest and brightest over the period
 *    
 * Requires underlying model
 * Position independent, time dependent
 */
class Pulsate : public Model {
  public:
    Pulsate(char const *name, float dimmest, float brightest, float dimSecs, float brightenSecs,
        std::shared_ptr<Model> model)
      : Model(name), model(model), dimmest(dimmest), brightest(brightest),
        dimSecs(dimSecs), brightenSecs(brightenSecs), periodSecs(dimSecs + brightenSecs) {}
    Color apply(float pos, float timeStamp);

  private:
    float const dimmest;
    float const brightest;
    float const dimSecs;
    float const brightenSecs;
    float const periodSecs;
    std::shared_ptr<Model> model;
};

// An animation that rotates or shifts lights to the left or right.
// The speed is specified in revs per second, which is the same
// how long it takes to move from position 0.0 to 1.0.
// Rotate wraps around so that once a color reaches 1.0, then
// continues at 0.0 again.
class RotateModel : public Model {
  public:
    enum Direction {
      UP, DOWN
    };
  
    RotateModel(char const *name, float revsPerSecond, Direction dir, std::shared_ptr<Model> model) 
      : Model(name), revsPerSecond(revsPerSecond), dir(dir), model(model) {}
    Color apply(float pos, float timeStamp);

  private:
    float revsPerSecond;
    Direction dir;
    std::shared_ptr<Model> model;
};

/***** COMPOSITES - Animations that are composites of other models *****/
extern std::shared_ptr<Model> makeDarkCrystal();
extern std::shared_ptr<Model> makeCrystal(
  Color upperColor, int upperSpeed,
  Color middleColor, int middleSpeed,
  Color lowerColor, int lowerSpeed);

#endif // __ANIMATIONS__
