#ifndef __RENDERER__
#define __RENDERER__

#include <memory>

#include "Model.h"
#include "lumos-arduino/lumos-arduino/Colors.h"

class Renderer {
  private:
    std::shared_ptr<Model> model;

    unsigned long startTimeMS;

  public:
    Renderer(std::shared_ptr<Model> model) : model(model), startTimeMS(millis()) { }
    Renderer() : model(NULL) { }
    void loop(int numPixels, std::function<void(int, Color)> setPixel);

    auto getModel() { return model; }
    void setModel(std::shared_ptr<Model> model);
};
 
#endif // __RENDERER__
