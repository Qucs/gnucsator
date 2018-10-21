simulator language=verilog
// This File is part of gnucap-qucs
// (C) 2018 Felix Salfelder
// GPLv3+

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
Vac #(.U(U) .f(f) .Phase(Phase)) V1(1 2i);
g_poly_2 #(.c(0.,1.,0.,0.,1.)) mul(1, 2j, 1, 2i, 3, 0);
vsource #(.dc(0.)) p(2j, 1);
ccvs #(.gain(1.)) HH(1,2,p);
endmodule;
// -------------------------------------------------------------------- //

// OpAmp:OP1 gnd _net0 _net4 G="1e6" Umax="15 V"
// module OpAmp(1, 2, 3);
// 	parameter G=1
// 	parameter Umax=15
// 	vcvs #(.gain(1.)) HH(1, 2, p);
// endmodule;
// hidemodule OpAmp
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

******************************************************************************
.verilog
parameter npn=1
parameter pnp=-1

// BJT:BC557B_1 _net4 _net1 _net2 _net1 Type="pnp" Is="3.834e-14" Nf="1.008"
// Nr="1.005" Ikf="0.08039" Ikr="0.047" Vaf="21.11" Var="32.02"
// Ise="1.219e-14" Ne="1.528" Isc="2.852e-13" Nc="1.28" Bf="344.4" Br="14.84"
// Rbm="1" Irb="1e-06" Rc="0.5713" Re="0.6202" Rb="1" Cje="1.23e-11"
// Vje="0.6106" Mje="0.378" Cjc="1.084e-11" Vjc="0.1022" Mjc="0.3563"
// Xcjc="0.6288" Cjs="0" Vjs="0.75" Mjs="0.333" Fc="0.8027" Tf="5.595e-10"
// Xtf="3.414" Vtf="5.23" Itf="0.1483" Tr="1e-32" Temp="26.85" Kf="0" Af="1"
// Ffe="1" Kb="0" Ab="1" Fb="1" Ptf="0" Xtb="0" Xti="3" Eg="1.11" Tnom="26.85"
// Area="1"
// BJT:T1 _net6 _net7 _net8 _net7 Type="pnp" Is="1e-16" Nf="1" Nr="1" Ikf="0"
// Ikr="0" Vaf="0" Var="0" Ise="0" Ne="1.5" Isc="0" Nc="2" Bf="100" Br="1"
// Rbm="0" Irb="0" Rc="0" Re="0" Rb="0" Cje="0" Vje="0.75" Mje="0.33" Cjc="0"
// Vjc="0.75" Mjc="0.33" Xcjc="1.0" Cjs="0" Vjs="0.75" Mjs="0" Fc="0.5"
// Tf="0.0" Xtf="0.0" Vtf="0.0" Itf="0.0" Tr="0.0" Temp="26.85" Kf="0.0"
// Af="1.0" Ffe="1.0" Kb="0.0" Ab="1.0" Fb="1.0" Ptf="0.0" Xtb="0.0" Xti="3.0"
// Eg="1.11" Tnom="26.85" Area="1.0"
module BJT(b, c, e, s);
	parameter Area;
	parameter Type;
	parameter Temp;
	parameter Is="3.834e-14";
	parameter Nf="1.008";
	parameter Nr="1.005";
	parameter Ikf="0.08039";
	parameter Ikr="0.047";
	parameter Vaf="21.11";
	parameter Var="32.02";
	parameter Ise="1.219e-14";
	parameter Ne="1.528";
	parameter Isc="2.852e-13";
	parameter Nc="1.28";
	parameter Bf="344.4";
	parameter Br="14.84";
	parameter Rbm="1";
	parameter Irb="1e-06";
	parameter Rc="0.5713";
	parameter Re="0.6202";
	parameter Rb="1";
	parameter Cje="1.23e-11";
	parameter Vje="0.6106";
	parameter Mje="0.378";
	parameter Cjc="1.084e-11";
	parameter Vjc="0.1022";
	parameter Mjc="0.3563";
	parameter Xcjc="0.6288";
	parameter Cjs="0";
	parameter Vjs="0.75";
	parameter Mjs="0.333";
	parameter Fc="0.8027";
	parameter Tf="5.595e-10";
	parameter Xtf="3.414";
	parameter Vtf="5.23";
	parameter Itf="0.1483";
	parameter Tr="1e-32";
	parameter Kf="0";
	parameter Af="1";
	parameter Ffe="1";
	parameter Kb="0";
	parameter Ab="1";
	parameter Fb="1";
	parameter Ptf="0";
	parameter Xtb="0";
	parameter Xti="3";
	parameter Eg="1.11";

	paramset mynpn npn;
		.is=  Is;
		.nf=  Nf;
		.nr=  Nr;
		.ikf= Ikf;
		.ikr= Ikr;
		.vaf= Vaf;
		.var= Var;
		.ise= Ise;
		.ne=  Ne;
		.isc= Isc;
		.nc=  Nc;
		.bf=  Bf;
		.br=  Br;
		.rbm= Rbm;
		.irb= Irb;
		.rc=  Rc;
		.re=  Re;
		.rb=  Rb;
		.cje= Cje;
		.vje= Vje;
		.mje= Mje;
		.cjc= Cjc;
		.vjc= Vjc;
		.mjc= Mjc;
		.xcjc=Xcjc
		.cjs= Cjs;
		.vjs= Vjs;
		.mjs= Mjs;
		.fc=  Fc;
		.tf=  Tf;
		.xtf= Xtf;
		.vtf= Vtf;
		.itf= Itf;
		.tr=  Tr;
		.kf=  Kf;
		.af=  Af;
		.ptf= Ptf;
		.xtb= Xtb;
		.xti= Xti;
		.eg=  Eg;
	endparamset
		// .fb=  Fb;
		// .ffe= Ffe;
		// .kb=  Kb;
		// .ab=  Ab;

	paramset mypnp pnp;
		.is=  Is;
		.nf=  Nf;
		.nr=  Nr;
		.ikf= Ikf;
		.ikr= Ikr;
		.vaf= Vaf;
		.var= Var;
		.ise= Ise;
		.ne=  Ne;
		.isc= Isc;
		.nc=  Nc;
		.bf=  Bf;
		.br=  Br;
		.rbm= Rbm;
		.irb= Irb;
		.rc=  Rc;
		.re=  Re;
		.rb=  Rb;
		.cje= Cje;
		.vje= Vje;
		.mje= Mje;
		.cjc= Cjc;
		.vjc= Vjc;
		.mjc= Mjc;
		.xcjc=Xcjc
		.cjs= Cjs;
		.vjs= Vjs;
		.mjs= Mjs;
		.fc=  Fc;
		.tf=  Tf;
		.xtf= Xtf;
		.vtf= Vtf;
		.itf= Itf;
		.tr=  Tr;
		.kf=  Kf;
		.af=  Af;
		.ptf= Ptf;
		.xtb= Xtb;
		.xti= Xti;
		.eg=  Eg;
	endparamset
		// .fb=  Fb;
		// .ffe= Ffe;
		// .kb=  Kb;
		//.ab=  Ab;
	
	// what are these?
	//"OFF"
	//"ICVBE"
	//"ICVCE"
	// workaround: select one of them.
	mynpn #(.area(Area*(1.+Type)*.5+1e-20) .temp(Temp)) p(c, b, e, s);
	mypnp #(.area(Area*(1.-Type)*.5+1e-20) .temp(Temp)) p(c, b, e, s);
endmodule // BJT

// d'oh. in qucsator, "MOSFET" is both, nFET and pFET.
parameter nfet=1
parameter pfet=-1

module MOSFET(g, d, s, b);
	parameter Vt0=.7;
	parameter Kp=50e-6;
	parameter Gamma=.96;
	parameter Phi=.5763;
	parameter Lambda=0;
	parameter Rd=0;
	parameter Rs=0;
	parameter Rg=0;
	parameter Is=0;
	parameter N=1.;
	parameter W=3.5u;
	parameter L=5.5u;
	parameter Ld=.5e-6;
	parameter Tox=50e-9;
	parameter Cgso=0.;
	parameter Cgdo=0.;
	parameter Cgbo=0.;
	parameter Cbd=0.;
	parameter Cbs=0.;
	parameter Pb=.7;
	parameter Mj=.5;
	parameter Fc=.5;
	parameter Cjsw=.05e-9;
	parameter Mjsw=.33;
	parameter Tt=0;
	parameter Nsub=1e16;
	parameter Nss=0;
	parameter Tpg=1;
	parameter Uo=600.;
	parameter Rsh=20;
	parameter Nrd=1;
	parameter Nrs=1;
	parameter Cj=1e-4;
	parameter Js=1e-8;
	parameter Ad=100.p;
	parameter As=100.p;
	parameter Pd=50.u;
	parameter Ps=50.u;
	parameter Kf=0;
	parameter Af=1;
	parameter Ffe=1; // ignored
	parameter Temp=26.85;
	parameter Tnom=26.85;

	paramset mynmos nmos;
		.level=1;
		.kp=Kp;
		.gamma=Gamma;
		.phi=Phi;
		.lambda=Lambda;
		.rd=Rd;
		.rs=Rs;
		.is=Is;
		.ld=Ld;
		.tox=Tox;
		.cgso=Cgso;
		.cgdo=Cgdo;
		.cgbo=Cgbo;
		.cbd=Cbd;
		.cbs=Cbs;
		.pb=Pb;
		.mj=Mj;
		.fc=Fc;
		.cjsw=Cjsw;
		.mjsw=Mjsw;
		.nsub=Nsub;
		.nss=Nss;
		.tpg=Tpg;
		.uo=Uo;
		.rsh=Rsh;
		.cj=Cj;
		.js=Js;
		.kf=Kf;
		.af=Af;
		.tnom=Tnom;
		.vto=Vt0;
	endparamset
	//.pd=Pd;
	//.tt=Tt;
	//.ps=Ps;
	//.rg=Rg;
	//.ad=Ad;
	//.as=As;
	//.nrd=Nrd;
	//.nrs=Nrs;

	paramset mypmos pmos;
		.level=1;
		.vto=Vt0;
		.kp=Kp;
		.gamma=Gamma;
		.phi=Phi;
		.lambda=Lambda;
		.rd=Rd;
		.rs=Rs;
		.is=Is;
		.ld=Ld;
		.tox=Tox;
		.cgso=Cgso;
		.cgdo=Cgdo;
		.cgbo=Cgbo;
		.cbd=Cbd;
		.cbs=Cbs;
		.pb=Pb;
		.mj=Mj;
		.fc=Fc;
		.cjsw=Cjsw;
		.mjsw=Mjsw;
		.nsub=Nsub;
		.nss=Nss;
		.tpg=Tpg;
		.uo=Uo;
		.rsh=Rsh;
		.cj=Cj;
		.js=Js;
		.kf=Kf;
		.af=Af;
		.tnom=Tnom;
	endparamset
	//.pd=Pd;
	//.tt=Tt;
	//.ps=Ps;
	//.rg=Rg;
	//.ad=Ad;
	//.as=As;
	//.nrd=Nrd;
	//.nrs=Nrs;
	
	// workaround: select one of them.
	// make the other one very narrow
	mynmos #(.w(W*(1.+Type)*.5), .l(L), .temp(Temp)) n(d, g, s, b);
	mypmos #(.w(W*(1.-Type)*.5), .l(L), .temp(Temp)) p(d, g, s, b);
endmodule // MOSFET

parameter on=1
parameter off=0
spice
.options noinsensitive
* Switch:S1 _net0 _net1 init="off" time="[1 ms]" Ron="0" Roff="1e12" Temp="26.85" MaxDuration="1e-6" Transition="spline"
*
.subckt Switch(1 2);
.parameter init=1
.parameter time=1m
.parameter Roff=1e12
.parameter Ron=0
V1 c 0 pulse rise={2*time} pv={2-6*init} iv={2*init}
.model  sss  sw  ( vt=0  vh=1  ron={Ron+1e-10}  roff=Roff)
S1 1 2 c 0 sss

.ends
******************************************************************************
* OpAmp:OP1 gnd _net0 _net4 G="1e6" Umax="15 V"
.subckt OpAmp 1 2 3
	.parameter G=1
	.parameter Umax=15
	E1 3 0 1 2 tanh gain=G limit=Umax
.ends
******************************************************************************

.simulator lang=verilog
`ifdef GNUCAP
hidemodule Diode
hidemodule Relais
hidemodule AM_Mod
hidemodule OpAmp
hidemodule BJT
hidemodule MOSFET
hidemodule Switch
`endif
simulator lang=acs
