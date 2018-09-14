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

hidemodule Amp

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
		.vt0=Vt0;
		.kp=Kp;
		.gamma=Gamma;
		.phi=Phi;
		.lambda=Lambda;
		.rd=Rd;
		.rs=Rs;
		.rg=Rg;
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
		.tt=Tt;
		.nsub=Nsub;
		.nss=Nss;
		.tpg=Tpg;
		.uo=Uo;
		.rsh=Rsh;
		.nrd=Nrd;
		.nrs=Nrs;
		.cj=Cj;
		.js=Js;
		.ad=Ad;
		.as=As;
		.pd=Pd;
		.ps=Ps;
		.kf=Kf;
		.af=Af;
		.temp=Temp;
		.tnom=Tnom;
	endparamset

	paramset mypmos pmos;
		.level=1;
		.vt0=Vt0;
		.kp=Kp;
		.gamma=Gamma;
		.phi=Phi;
		.lambda=Lambda;
		.rd=Rd;
		.rs=Rs;
		.rg=Rg;
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
		.tt=Tt;
		.nsub=Nsub;
		.nss=Nss;
		.tpg=Tpg;
		.uo=Uo;
		.rsh=Rsh;
		.nrd=Nrd;
		.nrs=Nrs;
		.cj=Cj;
		.js=Js;
		.ad=Ad;
		.as=As;
		.pd=Pd;
		.ps=Ps;
		.kf=Kf;
		.af=Af;
		.temp=Temp;
		.tnom=Tnom;
	endparamset
	
	// workaround: select one of them.
	mynmos #(.w(W*(1.+Type)*.5), .l(L)) n(d, g, s, b);
	mypmos #(.w(W*(1.-Type)*.5), .l(L)) p(d, g, s, b);
endmodule

simulator lang=spice
.options noinsensitive
* MUT:Tr1 _net0 _net1 gnd gnd L1="1 mH" L2="1 mH" k=".9"
.subckt MUT (a1 b1 b2 a2)
.parameter k=.9
.parameter L1=1m
.parameter L2=1m
K1 (Ll1 Ll2) {k}
Ll1 (a1 a2) {L1}
Ll2 (b1 b2) {L2}
.ends
.verilog

`ifdef GNUCAP
hidemodule Gyrator
hidemodule MOSFET
hidemodule MUT
`endif
