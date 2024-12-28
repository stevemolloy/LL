#include "acc_elements.h"

Drift make_drift(double len) {
  return (Drift){ .length = len };
}

Quad make_quad(double len, double K1) {
  return (Quad){
    .length = len,
    .K1 = K1,
  };
}

Sbend make_sbend(double len, double phi, double K1) {
  return (Sbend){
    .length = len,
    .K1 = K1,
    .angle = phi,
  };
}

Sextupole make_sext(double len, double K2) {
  return (Sextupole){
    .length = len,
    .K2 = K2,
  };
}

Octupole make_oct(double len, double K3) {
  return (Octupole){
    .length = len,
    .K3 = K3,
  };
}

Cavity make_cavity(double len, double voltage, double harmonic, double lag) {
  return (Cavity) {
    .length = len,
    .voltage = voltage,
    .harmonic = harmonic,
    .lag = lag,
  };
}
