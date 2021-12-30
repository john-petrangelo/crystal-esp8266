#include "src/lumos-arduino/lumos-arduino/Colors.h"

class ModelRunner {
  private:
    Model *model;

    unsigned long startTimeMS;

  public:
    ModelRunner();
    void loop(int numPixels, Pixels pixels);
};

ModelRunner::ModelRunner() : startTimeMS(millis()) {
  printf("ModelRunner::ModelRunner entered\n");

  GradientModel *gm = new GradientModel(8, RED, VIOLET, INDIGO, BLUE, GREEN, YELLOW, ORANGE, RED);
  RotateModel *rm = new RotateModel(gm, 0.3, RotateModel::UP);
  printf("ModelRunner::ModelRunner gm=%p rm=%p\n", gm, rm);
  
  model = rm;

  printf("ModelRunner::ModelRunner leaving\n");
}

void ModelRunner::loop(int numPixels, Pixels pixels) {
//  printf("ModelRunner::loop entered\n");

  unsigned long nowMS = millis();
  float timeStamp = (nowMS - startTimeMS) / 1000.0;
//  printf("ModelRunner::loop ms=%lu\n", timeStamp);
  for (int i = 0; i < numPixels; i++) {
    float pos = ((float)i) / (numPixels - 1);
    pixels[i] = model->apply(pos, timeStamp);
//    printf("ModelRunner::loop pixels[%d]=0x%06X (pos=%f)\n", i, pixels[i], pos);
  }

  printf("ModelRunner::loop leaving duration=%ums\n", millis() - nowMS);
}
