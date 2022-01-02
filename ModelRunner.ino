#include "src/lumos-arduino/lumos-arduino/Colors.h"

class ModelRunner {
  private:
    Model *model;

    unsigned long startTimeMS;

  public:
    ModelRunner(Model *model);
    ModelRunner() : model(NULL) { }
    void loop(int numPixels, Pixels pixels);

    void setModel(Model *model);
};

ModelRunner::ModelRunner(Model *model) : model(model), startTimeMS(millis()) { }

void ModelRunner::loop(int numPixels, Pixels pixels) {
  
  // If there's no model then there's nothing to do
  if (model == NULL) {
    return;
  }
  
  unsigned long nowMS = millis();
  float timeStamp = (nowMS - startTimeMS) / 1000.0;
//  Logger::logf("ModelRunner::loop looping ts=%f\n", timeStamp);
  for (int i = 0; i < numPixels; i++) {
    float pos = ((float)i) / (numPixels - 1);
    Logger::logf("ModelRunner::loop looping i=%d pos=%f ts=%f\n", i, pos, timeStamp);
    pixels[i] = model->apply(pos, timeStamp);
  }

//  Logger::logf("ModelRunner::loop leaving duration=%ums\n", millis() - nowMS);
}

// Give the ModelRunner a new model to run. The old model will be deleted.
void ModelRunner::setModel(Model *newModel) {
  delete model;
  model = newModel;
  startTimeMS = millis();
}
