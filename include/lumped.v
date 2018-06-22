simulator language=verilog
// This File is part of gnucap-qucs
// (C) 2018 Felix Salfelder
// GPLv3

// mapping qucsator names to actual devices
// "verilog devices" section

// Gyrator:X1 _net0 _net2 gnd gnd R="50 Ohm" Zref="50 Ohm"
// Zref is some sparam hack.. ignore for now
module Gyrator(1 2 3 4);
parameter R=50

ccvs #(.gain(R)) vs1(2 3i vp2);
vsource #(.dc(0)) vp1(3i 3);

ccvs #(.gain(R)) vs2(4i 1 vp1);
vsource #(.dc(0)) vp2(4i 4);
endmodule

hidemodule Gyrator
