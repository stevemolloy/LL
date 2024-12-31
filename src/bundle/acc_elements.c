#include "acc_elements.h"

Element make_drift(double len) {
  return (Element) { 
    .type = ELETYPE_DRIFT,
    .as.drift.length = len,
  };
}

Element make_marker(void) {
  return make_drift(0.0);
}

Element make_quad(double len, double K1) {
  return (Element) {
    .type = ELETYPE_QUAD,
    .as.quad.length = len,
    .as.quad.K1 = K1,
  };
}

Element make_sbend(double len, double phi, double K1) {
  return (Element) {
    .type = ELETYPE_SBEND,
    .as.sbend.length = len,
    .as.sbend.K1 = K1,
    .as.sbend.angle = phi,
  };
}

Element make_sext(double len, double K2) {
  return (Element) {
    .type = ELETYPE_SEXTUPOLE,
    .as.sextupole.length = len,
    .as.sextupole.K2 = K2,
  };
}

Element make_oct(double len, double K3) {
  return (Element) {
    .type = ELETYPE_OCTUPOLE,
    .as.octupole.length = len,
    .as.octupole.K3 = K3,
  };
}

Element make_cavity(double len, double voltage, double harmonic, double lag) {
  return (Element) {
    .type = ELETYPE_CAVITY,
    .as.cavity.length = len,
    .as.cavity.voltage = voltage,
    .as.cavity.harmonic = harmonic,
    .as.cavity.lag = lag,
  };
}

Line concat_two_elements(Element E1, Element E2) {
  Line retval = {0};
  SDM_ARRAY_PUSH(retval, E1);
  SDM_ARRAY_PUSH(retval, E2);
  
  return retval;
}

Line add_element_to_line(Line line, Element ele) {
  Line retval = {0};
  for (size_t i=0; i<line.length; i++) {
    SDM_ARRAY_PUSH(retval, line.data[i]);
  }
  SDM_ARRAY_PUSH(retval, ele);
  return retval;
}

Line add_line_to_element(Element ele, Line line) {
  Line retval = {0};
  SDM_ARRAY_PUSH(retval, ele);
  for (size_t i=0; i<line.length; i++) {
    SDM_ARRAY_PUSH(retval, line.data[i]);
  }
  return retval;
}

Line add_line_to_line(Line l1, Line l2) {
  Line retval = {0};
  for (size_t i=0; i<l1.length; i++) {
    SDM_ARRAY_PUSH(retval, l1.data[i]);
  }
  for (size_t i=0; i<l2.length; i++) {
    SDM_ARRAY_PUSH(retval, l2.data[i]);
  }
  return retval;
}

Line add_reversedline_to_element(Element ele, Line line) {
  Line retval = {0};
  SDM_ARRAY_PUSH(retval, ele);

  for (int i=line.length-1; i>=0; i--) {
    SDM_ARRAY_PUSH(retval, line.data[i]);
  }
  return retval;
}

Line add_reversedline_to_line(Line l1, Line l2) {
  Line retval = {0};
  for (size_t i=0; i<l1.length; i++) {
    SDM_ARRAY_PUSH(retval, l1.data[i]);
  }
  for (int i=l2.length-1; i>=0; i--) {
    SDM_ARRAY_PUSH(retval, l2.data[i]);
  }
  return retval;
}

Line line_times_int(Line line, int repeats) {
  Line retval = {0};
  for (int i=0; i<repeats; i++) {
    for (size_t j=0; j<line.length; j++) {
      SDM_ARRAY_PUSH(retval, line.data[j]);
    }
  }
  return retval;
}

Line int_times_line(int repeats, Line line) {
  Line retval = {0};
  for (int i=0; i<repeats; i++) {
    for (size_t j=0; j<line.length; j++) {
      SDM_ARRAY_PUSH(retval, line.data[j]);
    }
  }
  return retval;
}

