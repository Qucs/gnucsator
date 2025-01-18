simulator language=verilog
// This File is part of gnucap-qucs
// (C) 2018 Felix Salfelder
// GPLv3+

// mapping qucsator names to actual devices
// "logic devices" section
//
//AND:Y1 _net2 _net1 _net0 V="1 V" t="0" TR="10"
`if 0 // not yet
module proper_AND(a b y);
parameter V=1
parameter TR=1

`if 1
paramset mos logic; \
delay= 1n;\
rise= 1n;\
fall= 1n;\
rs= 100.;\
rw= 1.G;\
thh= 0.75;\
thl= 0.25;\
mr= 5.;\
mf= 5.;\
over=10k;\
vmax= 5.;\
vmin= 0.;\
endparamset
`endif

// notyet

endmodule
`endif

simulator lang=spice
.options noinsensitive

******************************************************************************
* problem: no disciplines yet.
* kludge: global logic model, treat them all the same.
* this defines a default discipline and default PWL connect modules
* the "delay" here is the time unit, as in "`timescale unit/dtmin"
* (to be continued)
.model qucs_logic logic ( delay= 1n  rise= 1n  fall= 1n  rs= 100.  rw= 1.G
+ thh= 0.9  thl= 0.1  mr= 5.  mf= 5.  over=10k vmax=1  vmin= 0. )
******************************************************************************
.verilog
module AND(y, a, b);
	parameter V=1;
	parameter TR=1;
	parameter t=0;
	and #(.delay(t), .model(qucs_logic)) g1(y, a, b);
endmodule
// ***************************************************************************
module NAND(y, a, b);
	parameter v=1;
	parameter tr=1;
	parameter t=0;
	nand #(.delay(t), .model(qucs_logic)) g1(y, a, b);
endmodule
// ***************************************************************************
module NOR(y, a, b);
	parameter v=1;
	parameter tr=1;
	parameter t=0;
	nor #(.delay(t), .model(qucs_logic)) g1(y, a, b);
endmodule
// ***************************************************************************
module XOR(y, a, b);
	parameter v=1;
	parameter tr=1;
	parameter t=0;
	xor #(.delay(t), .model(qucs_logic)) g1(y, a, b);
endmodule
// ***************************************************************************
module XNOR(y, a, b);
	parameter v=1;
	parameter tr=1;
	parameter t=0;
	xnor #(.delay(t), .model(qucs_logic)) g1(y, a, b);
endmodule
// ***************************************************************************
module OR(y, a, b);
	parameter v=1;
	parameter tr=1;
	parameter t=0;
	or #(.delay(t), .model(qucs_logic)) g1(y, a, b);
endmodule
// ***************************************************************************
module Inv(y, a);
	parameter v=1;
	parameter tr=1;
	parameter t=1;
	inv #(.delay(t), .model(qucs_logic)) g1(y, a);
endmodule
// ***************************************************************************
simulator lang=acs
