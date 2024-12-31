#ifndef _ACC_ELEMENTS_H
#define _ACC_ELEMENTS_H

#include <stdlib.h>
#include <stdio.h>

#include "sdm_lib.h"

#define C 299792458.0f
#define ELECTRON_MASS 510998.9499961642f
#define BEAM_DOFS 6
#define ERADIUS_TIMES_RESTMASS 0.959976365e-9
#define C_Q 3.83193864121903e-13

#define SIXBYSIX_IDENTITY (double[]){ \
    1, 0, 0, 0, 0, 0, \
    0, 1, 0, 0, 0, 0, \
    0, 0, 1, 0, 0, 0, \
    0, 0, 0, 1, 0, 0, \
    0, 0, 0, 0, 1, 0, \
    0, 0, 0, 0, 0, 1, \
  }

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

typedef enum {
  ELETYPE_DRIFT = 0,
  ELETYPE_QUAD,
  ELETYPE_SBEND,
  ELETYPE_MULTIPOLE,
  ELETYPE_SEXTUPOLE,
  ELETYPE_OCTUPOLE,
  ELETYPE_CAVITY,
} EleType;

typedef struct {
  EleType type;
  double R_matrix[BEAM_DOFS*BEAM_DOFS];
  double eta_prop_matrix[9];
  union {
    Drift drift;
    Quad quad;
    Sbend sbend;
    Multipole multipole;
    Sextupole sextupole;
    Octupole octupole;
    Cavity cavity;
  } as;
} Element;

typedef struct {
  size_t capacity;
  size_t length;
  Element *data;
} Line;

Element make_drift(double len);
Element make_quad(double len, double K1);
Element make_sbend(double len, double phi, double K1);
Element make_sext(double len, double K2);
Element make_oct(double len, double K3);
Element make_cavity(double len, double voltage, double harmonic, double lag);
Element make_marker(void);
Line concat_two_elements(Element E1, Element E2);
Line add_element_to_line(Line line, Element ele);
Line add_line_to_element(Element ele, Line line);
Line add_line_to_line(Line l1, Line l2);
Line add_reversedline_to_element(Element ele, Line line);
Line add_reversedline_to_line(Line l1, Line l2);
Line line_times_int(Line line, int repeats);
Line int_times_line(int repeats, Line line);

#endif // !_ACC_ELEMENTS_H

