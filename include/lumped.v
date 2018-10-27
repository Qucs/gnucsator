simulator language=verilog
// This File is part of gnucap-qucs
// (C) 2018 Felix Salfelder
// GPLv3+
//
// mapping qucsator names to actual devices
// "lumped components" section

module C(p, n);
	parameter C
	parameter V
	capacitor #(.c(C)) C1(p, n);
endmodule // C

module L(p, n);
	parameter L
	inductor #(.l(L)) L1(p, n);
endmodule // C

// Gyrator:X1 _net0 _net2 gnd gnd R="50 Ohm" Zref="50 Ohm"
// Zref is some sparam hack.. ignore for now
module Gyrator(1 2 3 4);
parameter R=50

ccvs #(.gain(R)) vs1(2 3i vp2);
vsource #(.dc(0)) vp1(3i 3);

ccvs #(.gain(R)) vs2(4i 1 vp1);
vsource #(.dc(0)) vp2(4i 4);
endmodule


// Amp:X1 _net0 _net2 G="10" Z1="50 Ohm" Z2="50 Ohm" NF="0 dB"
module Amp(1 2);
	parameter G=10;
	parameter Z1=50;
	parameter Z2=50;
	parameter NF=1;

	// almost?
	resistor #(.r(Z1)) r(2i 2);
	vcvs #(.gain(G)) vs1(2i 0 1 0);
endmodule



simulator lang=spice
.options noinsensitive
* MUT:Tr1 _net0 _net1 gnd gnd L1="1 mH" L2="1 mH" k=".9"
.subckt MUT (a1 b1 b2 a2)
.parameter k=.9
.parameter L1=1m
.parameter L2=1m
K1 (Ll1 Ll2) {k}
.inductor Ll1 (a1 a2) {L1}
.inductor Ll2 (b1 b2) {L2}
.ends

* MUT2:Tr2 _net5 _net6 _net7 _net8 _net9 _net10 L1="1 mH" L2="1 mH" L3="1 mH" k12="0.9" k13="0.9" k23="0.9"
* TODO: check port order
.subckt MUT2 (a1 b1 c1 c2 b2 a2)
.parameter k12=.9
.parameter k13=.9
.parameter k23=.9
.parameter L1=1m
.parameter L2=1m
.parameter L3=1m
K1 (Ll1 Ll2) {k12}
K2 (Ll2 Ll3) {k23}
K3 (Ll1 Ll3) {k13}
Ll1 (a1 a2) {L1}
Ll2 (b1 b2) {L2}
Ll3 (c1 c2) {L3}
.ends
.verilog

`ifdef GNUCAP
hidemodule C
hidemodule L
hidemodule Amp
hidemodule Gyrator
hidemodule MUT
hidemodule MUT2
`endif
