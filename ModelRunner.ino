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
  for (int i = 0; i < numPixels; i++) {
    float pos = ((float)i) / (numPixels - 1);
    pixels[i] = model->apply(pos, timeStamp);
  }
}

// Give the ModelRunner a new model to run. The old model will be deleted.
void ModelRunner::setModel(Model *newModel) {
  Logger::logf("ModelRunner::setModel old model=%p new model=%p\n", model, newModel);
  delete model;
  Logger::logf("ModelRunner::setModel after delete\n");
  model = newModel;
  startTimeMS = millis();
}
