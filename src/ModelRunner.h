#ifndef __MODELRUNNER__
#define __MODELRUNNER__

#include "Model.h"
#include "lumos-arduino/lumos-arduino/Colors.h"

class ModelRunner {
  private:
    Model *model;

    unsigned long startTimeMS;

  public:
    ModelRunner(Model *model) : model(model), startTimeMS(millis()) { }
    ModelRunner() : model(NULL) { }
    void loop(int numPixels, Pixels pixels);

    void setModel(Model *model);
};
 
#endif // __MODEL__
