#include "acc_elements.h"

Drift make_drift(float len) {
  return (Drift){ .length = len };
}

Quad make_quad(float len, float K1) {
  return (Quad){
    .length = len,
    .K1 = K1,
  };
}

