// *********************************************
// 'let' is a keyword that introduces a variable
// -- let <variable_name>: <variable_type>
// -- <variable_type> can be:
//   -- int
//   -- float
//   -- Drift
//   -- Quad
//   -- Bend
//   -- etc.
// *********************************************

let h_rf: int    = 176;
let c0: float    = 2.99792458e8;
let periods: int = 20;
let circumference: float     = 528.0/periods;

let d1:     Drift = Drift( L = 0.01 );
let d2:     Drift = Drift( L = 0.30311 - 0.1 );
let d3:     Drift = Drift( L = 0.40311 - 0.30311 );
let d4:     Drift = Drift( L = 0.075 - 0.0375 );
let twk:    Drift = Drift( L = 0.25 );
let d5:     Drift = Drift( L = 0.0375 );
let d6:     Drift = Drift( L = 1.302 - 0.25 );
let d7:     Drift = Drift( L = 0.045 );
let d8:     Drift = Drift( L = 0.125 - 0.1 );
let d9:     Drift = Drift( L = 0.26268 - 0.045 - 0.125 );
let d10:    Drift = Drift( L = 0.00608 );
let d11:    Drift = Drift( L = 0.1 );
let d12:    Drift = Drift( L = 0.025 );
let d13:    Drift = Drift( L = 0.118 - 0.1 );
let d14:    Drift = Drift( L = 0.161 - 0.118 );
let d15:    Drift = Drift( L = 2.55 - 0.161 );
let d_corr: Drift = Drift( L = 0.05);

let q1   : Quad = Quad( L = 0.25000, Phi =  0.00000, K1 =  4.79596 );
let q2   : Quad = Quad( L = 0.25000, Phi =  0.00000, K1 = -4.30427 );

let q3   : Bend = Bend( L = 0.15000, Phi = -0.04132, K1 =  3.09361 );
let r1   : Bend = Bend( L = 0.15000, Phi = -0.26652, K1 =  5.46047 );
let d2_0 : Bend = Bend( L = 0.36189, Phi =  1.13556, K1 = -1.15655 );
let d2_1 : Bend = Bend( L = 0.05000, Phi =  0.15289, K1 = -0.84188 );
let d2_2 : Bend = Bend( L = 0.05000, Phi =  0.15000, K1 = -0.82408 );
let d2_3 : Bend = Bend( L = 0.05000, Phi =  0.14486, K1 = -0.48802 );
let d2_4 : Bend = Bend( L = 0.05000, Phi =  0.13740, K1 =  0.09853 );
let d2_5 : Bend = Bend( L = 0.05000, Phi =  0.13389, K1 =  0.11139 );
let d1_u6: Bend = Bend( L = 0.05000, Phi = -0.20542, K1 =  0.00181 );
let d1_u5: Bend = Bend( L = 0.05000, Phi = -0.05352, K1 =  0.00071 );
let d1_u4: Bend = Bend( L = 0.05000, Phi =  0.11626, K1 =  0.00075 );
let d1_u3: Bend = Bend( L = 0.05000, Phi =  0.13522, K1 =  0.00080 );
let d1_u2: Bend = Bend( L = 0.05000, Phi =  0.10448, K1 =  0.00001 );
let d1_u1: Bend = Bend( L = 0.05000, Phi =  0.10292, K1 = -0.00014 );
let d1_0 : Bend = Bend( L = 0.20424, Phi =  0.45463, K1 = -0.36804 );
let d1_d1: Bend = Bend( L = 0.05000, Phi =  0.09087, K1 = -0.00157 );
let d1_d2: Bend = Bend( L = 0.05000, Phi =  0.08600, K1 = -0.00199 );
let d1_d3: Bend = Bend( L = 0.05000, Phi =  0.08373, K1 = -0.00170 );
let d1_d4: Bend = Bend( L = 0.05000, Phi =  0.09601, K1 = -0.00239 );
let d1_d5: Bend = Bend( L = 0.05000, Phi =  0.08976, K1 = -0.00255 );
let ch:    Bend = Bend( L = 0.05 );
let cv:    Bend = Bend( L = 0.05 );

let s1: Sextupole = Sextupole( L = 0.1,  K2 = -1.24426e+02 );
let s2: Sextupole = Sextupole( L = 0.05, K2 =  9.02251e+01 );
let s3: Sextupole = Sextupole( L = 0.05, K2 =  3.30631e+02 );
let s4: Sextupole = Sextupole( L = 0.1,  K2 = -2.95678e+02 );

let o1: Octupole = Octupole( L = 0.10000, K3 =  2.04859e+04 );
let o2: Octupole = Octupole( L = 0.10000, K3 = -2.06184e+04 );
let o3: Octupole = Octupole( L = 0.10000, K3 =  1.44110e+04 );

let cav: Cavity = Cavity( Frequency = c0/circumference*h_rf, Voltage = 2*1.50e6, HarNum = h_rf, Phi = 0.0 );

let begin: Drift = Drift(L=0.0);
let bpm:   Drift = Drift(L=0.0);
let gs:    Drift = Drift(L=0.0);
let ge:    Drift = Drift(L=0.0);

// ******************************************************************************************

let b_uc:   Line = Line(d2_0, d2_1, d2_2, d2_3, d2_4, d2_5);
let i_b_uc: Line = Line(d2_5, d2_4, d2_3, d2_2, d2_1, d2_0);
let b_mc:   Line = Line(d1_u6, d1_u5, d1_u4, d1_u3, d1_u2, d1_u1, d1_0, d1_d1, d1_d2, d1_d3, d1_d4, d1_d5);
let i_b_mc: Line = Line(d1_d5, d1_d4, d1_d3, d1_d2, d1_d1, d1_0, d1_u1, d1_u2, d1_u3, d1_u4, d1_u5, d1_u6);
let m_cell: Line = Line(
	  s2, d5, d4, q3, twk, ge, d6, gs, s1, d7, bpm, d8, ch, cv, d9, o3,
	  -b_mc, d10, q2, d11, o2, d12, q1, d12, o1, d13, ch, cv, d14, bpm, ge,
	  d15);
let half_cell: Line = Line(s3, d5, bpm, d4, r1, d3, cv, ch, d2, s4, d1, -b_uc);
let unit_cell: Line = Line(half_cell, b_uc, d1, s4, ge, d2, gs, 2 * d_corr, d3, r1, d4, d5, s3);
let sup_per: Line = Line(-m_cell, unit_cell, unit_cell, half_cell, -half_cell, -unit_cell, -unit_cell, m_cell);
let sp: Line = Line(begin, sup_per, cav);

// ************************************
// The following is not yet implemented
// ************************************

// ******************************
// Now let's look at some physics
// ******************************

let line_length: float = get_length_of_line(sp);
println("Line length = ", line_length, " m");

// println("Line contains {len(sp)} elements");
// println("Line length = {line_length(sp)} m");
// 
// println("Linear matrix for d1_u1:");
// print_matrix(d1_u1.linear_map);
// 
// println("Linear matrix for entire line:");
// print_matrix(sp.linear_map);
// 
// save_twiss("twiss_file.csv");

