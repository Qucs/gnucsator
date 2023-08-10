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
	capacitor #(.c(C)) dev(p, n);
endmodule // C

module L(p, n);
	parameter L
	inductor #(.l(L)) dev(p, n);
endmodule // C

module R(p, n);
	parameter R
	parameter Temp=26.85
	parameter Tnom=26.85
	parameter Tc1=0
	parameter Tc2=0

	// local?
	parameter dT={Temp-Tnom}

	resistor #(.r(R * (1. + dT*(Tc1 + dT*Tc2)))) dev(p, n);
endmodule // R

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

// is it lumped?
// TLIN:Line1 _net0 _net1 Z="50 Ohm" L="100 mm" Alpha="0 dB" Temp="26.85"
module TLIN (a, b)
parameter Z=50
parameter L=100m
parameter Alpha=1
parameter Temp=26.85

// local??
parameter c0=299792458.0
tline #(.z(Z), .f(c0), .nl(L) t(a 0 b 0);
endmodule

module Tr (outp inp inn outn);
parameter T=1.
CCCS #(.gain(T)) v(outp_ inp inn outp);
vcvs #(.gain(T) e(outp_ outn inp inn);
endmodule

* sTr:Tr2 _net4 _net5 gnd _net6 Output Output T1="1" T2="1"
* sTr:Tr1 _net0 _net1 gnd _net2 gnd gnd T1="1" T2="1"
module sTr (outp inp inn 4 5 outn);
parameter T1=1.
parameter T2=1.
CCCS #(.gain(T1)) v(outp_ inp inn outp);
vcvs #(.gain(T1) e(outp_ outn inp inn);

CCCS #(.gain(T2)) v(5_ inp inn 5);
vcvs #(.gain(T2) e(5_ 4 inp inn);
endmodule

module BiasT(1 2 3)
parameter C=1
parameter L=1
capacitor #(.c(C)) c1(1, 2);
inductor #(.l(L)) l1(3, 2);
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
