simulator language=verilog
// This File is part of gnucap-qucs
// (C) 2018 Felix Salfelder
// GPLv3

// mapping qucsator names to actual devices
// "logic devices" section
//
//AND:Y1 _net2 _net1 _net0 V="1 V" t="0" TR="10"
module proper_AND(a b y);
parameter V=1
parameter TR=1

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

// notyet

endmodule

simulator lang=spice
.options noinsensitive

******************************************************************************
.subckt AND(y a b);
.parameter V=1
.parameter TR=1
VV1 dd 0 V
.model  mos logic ( delay= 1n  rise= 1n  fall= 1n  rs= 100.  rw= 1.G
+ thh= 0.9  thl= 0.1  mr= 5.  mf= 5.  over=10k vmax=V  vmin= 0. )
U7  y 0 dd dd a b  mos and

.ends
******************************************************************************
.simulator lang=acs
