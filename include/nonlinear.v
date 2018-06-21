simulator language=verilog
// This File is part of gnucap-qucs
// (C) 2018 Felix Salfelder
// GPLv3

// mapping qucsator names to actual devices
// nonlinear stuff

// -------------------------------------------------------------------- //

// AM_Mod:V1 _net1 gnd _net2 U="1 V" f="1 Hz" Phase="0" m="1.0"
// Phase is broken in qucsator?
module AM_Mod(1, 2, 3);
parameter U=1
parameter f=1
parameter Phase=0
parameter m=1
Vsin #(.U(U) .f(f) .Phase(Phase)) V1(1 2i);
g_poly_2 #(.c(0.,1.,0.,0.,1.)) mul(1, 2j, 1, 2i, 3, 0);
vsource #(.dc(0.)) p(2j, 1);
ccvs #(.gain(1.)) HH(1,2,p);
endmodule;

hidemodule AM_Mod

// -------------------------------------------------------------------- //

module Diode(1 2);
parameter N=1
parameter Cj0=15f
parameter M=0.5
parameter Vj=0.7
* Fc="0.5" Cp="0.0 fF" Isr="0.0" Nr="2.0"
parameter Rs=0.
* Tt="0.0 ps" Ikf="0"
* Kf="0.0" Af="1.0" Ffe="1.0" Bv="0" Ibv="1 mA" Temp="26.85" Xti="3.0"
* Eg="1.11" Tbv="0.0" Trs="0.0" Ttt1="0.0" Ttt2="0.0" Tm1="0.0" Tm2="0.0"
* Tnom="26.85" Area="1.0"
parameter Is=1e-15

paramset mydiode d;\
 .is=Is; \
 .rs=Rs; \
 .n=N; \
 .tt= 0.; \
 .cjo=Cj0; \
 .vj=Vj; \
 .m=M; \
endparamset

mydiode #(.area(1)) d1(2 1);
endmodule

simulator lang=spice
.options noinsensitive

******************************************************************************
* TODO: verilog
.subckt Relais(1 2 3 4);
.parameter Vt=.5
.parameter Vh=.1
* WEIRD PORT ORDER (intentional)

.model  sss  sw  ( vt=Vt  vh=Vh  ron=1m  roff=1.E+12 )
S1 1 3 2 4 sss

.ends

* Switch:S1 _net0 _net1 init="off" time="[1 ms]" Ron="0" Roff="1e12" Temp="26.85" MaxDuration="1e-6" Transition="spline"
*
.subckt Switch(1 2);
.parameter time=1m
.parameter Roff=1e12
.parameter Ron=0
V1 c 0 pulse rise={2*time} pv=2 iv=0
.model  sss  sw  ( vt=.5  vh=.5  ron={Ron+1e-20}  roff=Roff)
S1 1 2 c 0 sss

.ends
******************************************************************************

.simulator lang=acs
* hide in listings.
hidemodule Diode
hidemodule Relais
