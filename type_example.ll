let ele1: Drift = Drift(L = 0.5);
let ele2: Drift = Drift(L=0.1);
let ele3: Drift = Drift(L = 0.5);
let ele4: Drift = Drift(L=0.1);
let ele5: Quad = Quad(L=0.5, K1=1.0);

let l1: Line = ele2 + ele3;
let l2: Line = 2 * l1 + ele4;
let l3: Line = ele1 + l2;
let l4: Line = l2 - l3;
let l5: Line = 2 * l4;
