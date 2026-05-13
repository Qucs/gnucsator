options noinsensitive
// This File is part of gnucap-qucs
// (C) 2018 Felix Salfelder
// GPLv3+
//
// mapping qucsator names to actual devices
// "lumped components" section

module C(p, n);
	electrical p, n;
	inout p, n;
	parameter real C = 0.0;
	parameter real V = 0.0;

	capacitor #(.c(C)) dev(p, n);
endmodule // C

module L(p, n);
	electrical p, n;
	inout p, n;
	parameter real L = 0.0;

	inductor #(.l(L)) dev(p, n);
endmodule // C

module R(p, n);
	electrical p, n;
	inout p, n;
	parameter real R = 1.0;
	parameter real Temp = 26.85;
	parameter real Tnom = 26.85;
	parameter real Tc1 = 0.0;
	parameter real Tc2 = 0.0;
	localparam real dT = Temp - Tnom;

	resistor #(.r(R * (1. + dT*(Tc1 + dT*Tc2)))) dev(p, n);
endmodule // R

// Gyrator:X1 _net0 _net2 gnd gnd R="50 Ohm" Zref="50 Ohm"
// Zref is some sparam hack.. ignore for now

module Gyrator(\1 , \2 , \3 , \4 );
  electrical \1 , \2 , \3 , \4 ;
  inout \1 , \2 , \3 , \4 ;
  parameter real R = 1.0;
  parameter real Zref = 50.0;

  vccs #(1/R) cs1(\2 , \3 , \4 , \1 );
  vccs #(1/R) cs2(\1 , \4 , \2 , \3 );
endmodule

// old gyrator, seems numerically unstable
// use with caution.
module Gyrator1(\1 , \2 , \3 , \4 );
  electrical \1 , \2 , \3 , \4 ;
  inout \1 , \2 , \3 , \4 ;
  parameter real R = 50.0;
  parameter real Zref R = 50.0;

  ccvs #(.gain(R)) vs1(\3i , \2 , vp2);
  vsource #(.dc(0)) vp1(\3 , \3i );
  
  ccvs #(.gain(R)) vs2(\1 ,\4i , vp1);
  vsource #(.dc(0)) vp2(\4 , \4i );
endmodule


// Amp:X1 _net0 _net2 G="10" Z1="50 Ohm" Z2="50 Ohm" NF="0 dB"
module Amp(\1 , \2 );
	electrical \1 , \2 , gnd;
	inout \1 , \2 ;
	ground gnd;
	parameter real G = 10.0;
	parameter real Z1 = 50.0;
	parameter real Z2 = 50.0;
	parameter real NF = 1.0;

	// almost?
	resistor #(.r(Z1)) r(\2i , \2 );
	vcvs #(.gain(G)) vs1(\2i , gnd, \1 , gnd);
endmodule

// is it lumped?
// TLIN:Line1 _net0 _net1 Z="50 Ohm" L="100 mm" Alpha="0 dB" Temp="26.85"
//                                           is ignored  ^^
module TLIN (t1, t2)
  electrical t1, t2, gnd;
  inout t1, t2;
  ground gnd;
  parameter real Z = 50.0;
  parameter real L = 0.1;
  parameter real Alpha = 0.0;
  parameter real Temp = 26.85;
  localparam real c0 = 299792458.0;

  tline #(.z(Z), .td(1./c0), .len(L), .alpha(Alpha)) t2(.t1(t1) , .b1(gnd) , .t2(t2) , .b2(gnd) );
endmodule

module TLIN4P (t1, t2, b2, b1);
  electrical t1, t2, b1, b2;
  inout t1, t2, b1, b2;
  parameter real Z = 50.0;
  parameter real L = 0.1;
  parameter real Alpha = 0.0;
  parameter real Temp = 26.85;
  localparam real c0 = 299792458.0;

  tline #(.z(Z), .td(1./c0), .len(L), .alpha(Alpha)) t4(.t1(t1), .t2(t2), .b1(b1), .b2(b2));
endmodule

module Tr (outp, inp, inn, outn);
  electrical outp, inp, inn, outn;
  inout outp, inp, inn, outn;
  parameter real T=1.0;

  CCCS #(.G(T)) v(outp_, inp, inn, outp);
  vcvs #(.gain(T)) e(outp_, outn, inp, inn);
endmodule

* sTr:Tr2 _net4 _net5 gnd _net6 Output Output T1="1" T2="1"
* sTr:Tr1 _net0 _net1 gnd _net2 gnd gnd T1="1" T2="1"
module sTr (outp, inp, inn, \4 , \5 , outn);
  electrical outp, inp, inn, \4 , \5 , outn;
  inout outp, inp, inn, \4 , \5 , outn;
  parameter real T1 = 1.0;
  parameter real T2 = 1.0;

  CCCS #(.G(T1)) v(outp_ inp inn outp);
  vcvs #(.gain(T1)) e(outp_ outn inp inn);

  CCCS #(.G(T2)) v(\5_ , inp, inn, \5 );
  vcvs #(.gain(T2)) e(\5_ , \4 , inp, inn);
endmodule

module BiasT(\1 , \2 , \3 )
  electrical \1 , \2 , \3 ;
  inout \1 , \2 , \3 ;
  parameter real C = 1.0;
  parameter real L = 1.0;

  capacitor #(.c(C)) c1(\1 , \2 );
  inductor #(.l(L)) l1(\3 , \2 );
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
