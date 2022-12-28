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
CCVS h(2j 1 2 1);
endmodule;
// -------------------------------------------------------------------- //
// OpAmp:OP1 gnd _net0 _net4 G="1e6" Umax="15 V"
// module OpAmp(1, 2, 3);
// 	parameter G=1
// 	parameter Umax=15
// 	vcvs #(.gain(1.)) HH(1, 2, p);
// endmodule;
// -------------------------------------------------------------------- //
simulator lang=spice
.options noinsensitive
.subckt spice_diode 1 2
 .parameter area=1
 .parameter N cjo
 .parameter M, vj
 .parameter rs
 .parameter Is=1e-15
 .parameter tt=1e-15

 .model mydiode d ( is=Is rs=rs n=N tt=tt cjo=cjo vj=vj m=M )
  D1 1 2 mydiode area=area
.ends

.verilog

`if 1
// TODO: this is actually a paramset
module Diode(1 2);
parameter N=1
parameter Cj0=15f
parameter M=0.5
parameter Vj=0.7
parameter Rs=0.
parameter Is=1e-15
parameter Area=1.
parameter Tt=0.

// ignored, but somehow used in .net files revisit later.
parameter Fc=.5 Cp=0. Isr=0. Nr=2.
parameter Ikf=0 Kf=0. Af=1. Ffe=1. Bv=0. Ibv=1m Temp=26.85 Xti=3.
parameter Eg=1.11 Tbv=0. Trs=0. Ttt1=0. Ttt2=0. Tm1=0. Tm2=0.
parameter Tnom=26.85

  spice_diode #( .area(Area) .Is(Is) .rs(Rs) .N(N) .cjo(Cj0) .vj(Vj) .M(M) .tt(Tt)) d1(2 1);
endmodule
`else
paramset Diode spice_diode
  parameter N=1 Cj0=15f M=0.5 Vj=0.7 Rs=0.  Is=1e-15 area=1
 .area=area
 .Is=Is
 .Rs=Rs
 .N=N
 .Cj0=Cj0
 .Vj=Vj
 .M=M
endparamset
`endif

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
.parameter npn=1
.parameter pnp=-1

* wrap "spice model" into subckt to expose device parameters.
.subckt spice_npn (b, c, e, s)
 .parameter area=1
 .parameter temp is nf nr ikf ikr vaf var ise ne isc nc bf br rbm irb rc
          + re rb cje vje mje cjc vjc mjc xcjc cjs vjs mjs fc tf xtf vtf itf
          + tr kf af ffe kb ab fb ptf xtb xti eg

 .model mynpn npn ( is=is nf=nf nr=nr ikf=ikf ikr=ikr vaf=vaf
      + var= var ise= ise ne=ne isc=isc nc=nc bf=bf br=br rbm=rbm
      + irb=irb rc=rc re=re rb=rb cje=cje vje=vje mje=mje cjc=cjc vjc=vjc
      + mjc=mjc xcjc=xcjc cjs=cjs vjs=vjs mjs=mjs fc=fc tf=tf xtf=xtf vtf=vtf
      + itf=itf tr=tr kf=kf af=af ptf=ptf xtb=xtb xti=xti eg=eg )

  Q1 (c, b, e, s) mynpn area=area
.ends

.subckt spice_pnp (b, c, e, s)
 .parameter area=1
 .parameter temp is nf nr ikf ikr vaf var ise ne isc nc bf br rbm irb rc
          + re rb cje vje mje cjc vjc mjc xcjc cjs vjs mjs fc tf xtf vtf itf
          + tr kf af ffe kb ab fb ptf xtb xti eg

 .model mypnp pnp ( is=is nf=nf nr=nr ikf=ikf ikr=ikr vaf=vaf
      + var= var ise= ise ne=ne isc=isc nc=nc bf=bf br=br rbm=rbm
      + irb=irb rc=rc re=re rb=rb cje=cje vje=vje mje=mje cjc=cjc vjc=vjc
      + mjc=mjc xcjc=xcjc cjs=cjs vjs=vjs mjs=mjs fc=fc tf=tf xtf=xtf vtf=vtf
      + itf=itf tr=tr kf=kf af=af ptf=ptf xtb=xtb xti=xti eg=eg )
*		// .fb=  Fb;
*		// .ffe= Ffe;
*		// .kb=  Kb;
*		// .ab=  Ab;
  Q1 (c, b, e, s) mypnp area=area
.ends
******************************************************************************
.verilog
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
//
// TODO: these are two paramsets
module BJT (b, c, e, s);
  parameter Area=1;
  parameter Type;
  parameter Temp=26.85 Is=3.834e-14 Nf=1.008 Nr=1.005 Ikf=0.08039 Ikr=0.047 Vaf=21.11 Var=32.02;
  parameter Ise=1.219e-14 Ne=1.528 Isc=2.852e-13 Nc=1.28 Bf=344.4 Br=14.84 Rbm=1 Irb=1e-06 Rc=0.5713;
  parameter Re=0.6202 Rb=1 Cje=1.23e-11 Vje=0.6106 Mje=0.378 Cjc=1.084e-11 Vjc=0.1022 Mjc=0.3563;
  parameter Xcjc=0.6288 Cjs=0 Vjs=0.75 Mjs=0.333 Fc=0.8027 Tf=5.595e-10 Xtf=3.414 Vtf=5.23 Itf=0.1483;
  parameter Tr=1e-32 Kf=0 Af=1 Ffe=1 Kb=0 Ab=1 Fb=1 Ptf=0 Xtb=0 Xti=3 Eg=1.11;
  parameter Tnom;

// .fb=  Fb;
// .ffe= Ffe;
// .kb=  Kb;
//.ab=  Ab;

// what are these?
//"OFF"
//"ICVBE"
//"ICVCE"
// workaround: select one of them.
  spice_npn #(.is(Is) .nf(  Nf) .nr(  Nr) .ikf( Ikf) .ikr( Ikr) .vaf( Vaf) .var( Var) .ise( Ise) .ne(  Ne) \
      .isc( Isc) .nc(  Nc) .bf(  Bf) .br(  Br) .rbm( Rbm) .irb( Irb) .rc(  Rc) .re(  Re) .rb(  Rb) .cje( Cje) \
      .vje( Vje) .mje( Mje) .cjc( Cjc) .vjc( Vjc) .mjc( Mjc) .xcjc(Xcjc) .cjs( Cjs) .vjs( Vjs) .mjs( Mjs) \
      .fc(  Fc) .tf(  Tf) .xtf( Xtf) .vtf( Vtf) .itf( Itf) .tr(  Tr) .kf(  Kf) .af(  Af) .ptf( Ptf) \
      .xtb( Xtb) .xti( Xti) .eg(  Eg) .area(Area*(1.+Type)*.5) .temp(Temp)) npn(b, c, e, s);

  spice_pnp #(.is(Is) .nf(  Nf) .nr(  Nr) .ikf( Ikf) .ikr( Ikr) .vaf( Vaf) .var( Var) .ise( Ise) .ne(  Ne) \
      .isc( Isc) .nc(  Nc) .bf(  Bf) .br(  Br) .rbm( Rbm) .irb( Irb) .rc(  Rc) .re(  Re) .rb(  Rb) .cje( Cje) \
      .vje( Vje) .mje( Mje) .cjc( Cjc) .vjc( Vjc) .mjc( Mjc) .xcjc(Xcjc) .cjs( Cjs) .vjs( Vjs) .mjs( Mjs) \
      .fc(  Fc) .tf(  Tf) .xtf( Xtf) .vtf( Vtf) .itf( Itf) .tr(  Tr) .kf(  Kf) .af(  Af) .ptf( Ptf) \
      .xtb( Xtb) .xti( Xti) .eg(  Eg) .area(Area*(1.-Type)*.5) .temp(Temp)) pnp(b, c, e, s);
endmodule // BJT

// d'oh. in qucsator, "MOSFET" is both, nFET and pFET.
parameter nfet=1
parameter pfet=-1

spice
.options noinsensitive

.subckt spice_nmos(d, g, s, b);
 .parameter vto=.7, kp=50e-6, gamma=.96, phi=.5763, lambda=0, rd=0, rs=0, rg=0, is=0, n=1., w=3.5u, l=5.5u
 .parameter ld=.5e-6, tox=50e-9, cgdo=0., cgso=0., cgbo=0., cbd=0., cbs=0., pb=.7, mj=.5, fc=.5, cjsw=.05e-9
 .parameter mjsw=.33, tt=0, nsub=1e16, nss=0, tpg=1, uo=600., rsh=20, nrd=1, nrs=1, cj=1e-4, js=1e-8
 .parameter ad=100.p, as=100.p, pd=50.u, ps=50.u, kf=0, af=1, temp=26.85, tnom=26.85
 .parameter m

 * ignored
 .parameter ffe=1

 .model mynmos nmos
   + level=1 kp=kp gamma=gamma phi=phi lambda=lambda rd=rd rs=rs is=is ld=ld tox=tox
   + cgso=cgso cgbo=cgbo cgdo=cgdo cbd=cbd cbs=cbs pb=pb mj=mj fc=fc cjsw=cjsw mjsw=mjsw
   + nsub=nsub nss=nss tpg=tpg uo=uo rsh=rsh cj=cj js=js kf=kf af=af tnom=tnom vto=vto

  *.pd=Pd; .tt=Tt; .ps=Ps; .rg=Rg; .ad=Ad; .as=As; .nrd=Nrd; .nrs=Nrs;

  * M d g s b mynmos l=l w=w ad=ad as=as pd=pd ps=ps nrd=nrd nrs=nrs m=m temp=temp
  M d g s b mynmos l=l w=w m=m temp=temp
.ends

.subckt spice_pmos(d, g, s, b);
 .parameter vto=.7, kp=50e-6, gamma=.96, phi=.5763, lambda=0, rd=0, rs=0, rg=0, is=0, n=1., w=3.5u, l=5.5u
 .parameter ld=.5e-6, tox=50e-9, cgdo=0., cgso=0., cgbo=0., cbd=0., cbs=0., pb=.7, mj=.5, fc=.5, cjsw=.05e-9
 .parameter mjsw=.33, tt=0, nsub=1e16, nss=0, tpg=1, uo=600., rsh=20, nrd=1, nrs=1, cj=1e-4, js=1e-8
 .parameter ad=100.p, as=100.p, pd=50.u, ps=50.u, kf=0, af=1, temp=26.85, tnom=26.85
 .parameter m

 * ignored
 .parameter ffe=1

 .model mypmos pmos
   + level=1 kp=kp gamma=gamma phi=phi lambda=lambda rd=rd rs=rs is=is ld=ld tox=tox
   + cgso=cgso cgbo=cgbo cgdo=cgdo cbd=cbd cbs=cbs pb=pb mj=mj fc=fc cjsw=cjsw mjsw=mjsw
   + nsub=nsub nss=nss tpg=tpg uo=uo rsh=rsh cj=cj js=js kf=kf af=af tnom=tnom vto=vto

  *.pd=Pd; .tt=Tt; .ps=Ps; .rg=Rg; .ad=Ad; .as=As; .nrd=Nrd; .nrs=Nrs;

  * M d g s b mypmos l=l w=w ad=ad as=as pd=pd ps=ps nrd=nrd nrs=nrs m=m temp=temp
  M d g s b mypmos l=l w=w m=m temp=temp
.ends

.verilog

// TODO: this is a paramset
module MOSFET(g, d, s, b);
  parameter Vt0=.7, Kp=50e-6, Gamma=.96, Phi=.5763, Lambda=0, Rd=0, Rs=0, Rg=0, Is=0, N=1., W=3.5u, L=5.5u;
  parameter Ld=.5e-6, Tox=50e-9, Cgso=0., Cgdo=0., Cgbo=0., Cbd=0., Cbs=0., Pb=.7, Mj=.5, Fc=.5, Cjsw=.05e-9;
  parameter Mjsw=.33, Tt=0, Nsub=1e16, Nss=0, Tpg=1, Uo=600., Rsh=20, Nrd=1, Nrs=1, Cj=1e-4, Js=1e-8;
  parameter Ad=100.p, As=100.p, Pd=50.u, Ps=50.u, Kf=0, Af=1, Temp=26.85, Tnom=26.85;
  parameter Ffe=1; // ignored

	parameter Type;

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

  // workaround: select one of them.
  spice_nmos #(.w(W), .m((1.+Type)*.5), .l(L), .temp(Temp) \
    .kp(Kp) .gamma(Gamma) .phi(Phi) .lambda(Lambda) .rd(Rd) .rs(Rs) .is(Is) .ld(Ld) .tox(Tox) \
    .cgso(Cgso) .cgdo(Cgdo) .cgbo(Cgbo) .cbd(Cbd) .cbs(Cbs) .pb(Pb) .mj(Mj) .fc(Fc) .cjsw(Cjsw) .mjsw(Mjsw) \
    .nsub(Nsub) .nss(Nss) .tpg(Tpg) .uo(Uo) .rsh(Rsh) .cj(Cj) .js(Js) .kf(Kf) .af(Af) .tnom(Tnom) .vto(Vt0) \
   ) n(d, g, s, b);

  spice_pmos #(.w(W), .m((1.-Type)*.5), .l(L), .temp(Temp) \
    .kp(Kp) .gamma(Gamma) .phi(Phi) .lambda(Lambda) .rd(Rd) .rs(Rs) .is(Is) .ld(Ld) .tox(Tox) \
    .cgso(Cgso) .cgdo(Cgdo) .cgbo(Cgbo) .cbd(Cbd) .cbs(Cbs) .pb(Pb) .mj(Mj) .fc(Fc) .cjsw(Cjsw) .mjsw(Mjsw) \
    .nsub(Nsub) .nss(Nss) .tpg(Tpg) .uo(Uo) .rsh(Rsh) .cj(Cj) .js(Js) .kf(Kf) .af(Af) .tnom(Tnom) .vto(Vt0) \
    ) p(d, g, s, b);
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
.model  sss  sw  ( vt=0  vh=1  ron={if(Ron,Ron,1e-10)}  roff=Roff)
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

.simulator lang=acs
