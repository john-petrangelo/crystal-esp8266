#ifndef __ANIMATIONS__
#define __ANIMATIONS__

#include "lumos-arduino/lumos-arduino/Colors.h"

#include "Model.h"

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
  
    RotateModel(char const *name, float revsPerSecond, Direction dir, Model *model) 
      : Model(name), revsPerSecond(revsPerSecond), dir(dir), model(model) {}
    Color apply(float pos, float timeStamp);
    ~RotateModel() { delete model; model = NULL; }

  private:
    float revsPerSecond;
    Direction dir;
    Model *model;
};

#endif // __ANIMATIONS__
