#include <functional>
#include "ModelRunner.h"

void ModelRunner::loop(int numPixels, std::function<void(int, Color)> setPixel) {
  // If there's no model then there's nothing to do
  if (model == NULL) {
    return;
  }
  
  unsigned long nowMS = millis();
  float timeStamp = (nowMS - startTimeMS) / 1000.0;
  for (int i = 0; i < numPixels; i++) {
    float pos = ((float)i) / (numPixels - 1);
    Color color = model->apply(pos, timeStamp);
    setPixel(i, color);
  }
}

// Give the ModelRunner a new model to run. The old model will be deleted.
void ModelRunner::setModel(Model *newModel) {
  delete model;
  model = newModel;
  startTimeMS = millis();
}
