#ifndef __MODELRUNNER__
#define __MODELRUNNER__

#include <memory>

#include "Model.h"
#include "lumos-arduino/lumos-arduino/Colors.h"

class ModelRunner {
  private:
    std::shared_ptr<Model> model;

    unsigned long startTimeMS;

  public:
    ModelRunner(std::shared_ptr<Model> model) : model(model), startTimeMS(millis()) { }
    ModelRunner() : model(NULL) { }
    void loop(int numPixels, std::function<void(int, Color)> setPixel);

    std::shared_ptr<Model> getModel() { return model; }
    void setModel(std::shared_ptr<Model> model);
};
 
#endif // __MODEL__
