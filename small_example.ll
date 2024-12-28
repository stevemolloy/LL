let h_rf: int    = 176;
let c0: float    = 2.99792458e8;
let periods: int = 20;
let C: float     = 528.0/periods;

let bpm: Marker;
let d1: Drift = Drift( L = 0.01 );
let q1: Quad = Quad( L = 0.25000, K1 =  4.79596 );
let d2_5: Bend = Bend( L = 0.05000, Phi =  0.13389, K1 =  0.11139 );
let s3: Sextupole = Sextupole( L = 0.05, K2 =  3.30631e+02 );
let o2: Octupole = Octupole( L = 0.10000, K3 = -2.06184e+04 );
let cav: Cavity = Cavity( Frequency = c0/C*h_rf, Voltage = 2*1.50e6, HarNum = h_rf, Phi = 0.0 );

println(1, 2, 3, 4, 5, 6, 7);

println(periods);
println(C);
println("C is equal to {C}");

