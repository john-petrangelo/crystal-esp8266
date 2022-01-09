#include "Animations.h"
#include "Model.h"

Model *makeDemo1() {
  Model *grad_left = new GradientModel("grad-left", RED, YELLOW);
  Model *grad_right = new GradientModel("grad-right", BLUE, GREEN);

  Model *rot_left = new RotateModel("rotate down", 4.0, RotateModel::DOWN, grad_left);
  Model *rot_right = new RotateModel("rotate up", 1.0, RotateModel::UP, grad_right);

  Model *map_left = new MapModel("map left", 0.0, 0.2, 0.0, 1.0, rot_left);
  Model *map_right = new MapModel("map right", 0.2, 1.0, 0.0, 1.0, rot_right);

  Model *window = new WindowModel("window", 0.0, 0.2, map_left, map_right);
  Model *rot_window = new RotateModel("rotate window", 0.5, RotateModel::DOWN, window);

  return rot_window;
};

Model *makeDemo2() {
  Model *gradient = new GradientModel("grad", BLUE, RED);
  Model *rot_grad = new RotateModel("Rotate Gradient", 2.0, RotateModel::UP, gradient);
  Model *rev_grad = new ReverseModel(rot_grad);

  Model *map_left = new MapModel("map left", 0.0, 0.5, 0.0, 1.0, rot_grad);
  Model *map_right = new MapModel("map right", 0.5, 1.0, 0.0, 1.0, rev_grad);

  Model *window = new WindowModel("window", 0.0, 0.5, map_left, map_right);

  return window;
}
