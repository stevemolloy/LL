#ifndef _ACC_ELEMENTS_H
#define _ACC_ELEMENTS_H

typedef struct {
  double length;
} Drift;

typedef Drift Marker;

typedef struct {
  double length;
  double K1;
} Quad;

typedef struct {
  double length;
  double angle;
  double K1;
  double E1;
  double E2;
} Sbend;

// Multipole, K3L= -38933.64000000; 
typedef struct {
  double length;
  double K1L;
  double K2L;
  double K3L;
} Multipole;

//Sextupole,  L= 0.10000000, K2= -272.67400000 ;
typedef struct {
  double length;
  double K2;
} Sextupole;

typedef struct {
  double length;
  double K3;
} Octupole;

// CAV : RFCavity, L=0.37800000,VOLT=0.09000000, harm=176, lag=0.0; 
typedef struct {
  double length;
  double voltage;
  double harmonic;
  double lag;
} Cavity;

Drift make_drift(double len);
Quad make_quad(double len, double K1);
Sbend make_sbend(double len, double phi, double K1);
Sextupole make_sext(double len, double K2);
Octupole make_oct(double len, double K3);
Cavity make_cavity(double len, double voltage, double harmonic, double lag);

#endif // !_ACC_ELEMENTS_H

