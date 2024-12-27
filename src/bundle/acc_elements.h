#ifndef _ACC_ELEMENTS_H
#define _ACC_ELEMENTS_H

typedef struct {
  double length;
} Drift;

typedef struct {
  double length;
  double K1;
} Quad;

Drift make_drift(float len);
Quad make_quad(float len, float K1);

#endif // !_ACC_ELEMENTS_H

