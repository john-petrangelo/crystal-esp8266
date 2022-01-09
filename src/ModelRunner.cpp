#include "ModelRunner.h"

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
  delete model;
  model = newModel;
  startTimeMS = millis();
}
