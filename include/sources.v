simulator language=verilog
// This File is part of gnucap-qucs
// (C) 2018 Felix Salfelder
// GPLv3+

// mapping qucsator names to actual devices
// "sources" section

module Idc(1 2);
parameter I=1m
isource #(.dc(I)) dev(2 1);
endmodule

module Vdc(p n);
// it's missing a probe
	parameter U=1m
	parameter Temp
	vsource #(.dc(U)) dev(p n);
endmodule

module VCCS(1 2 3 4);
parameter G=1
vccs #(.gm(G)) v(2 3 1 4);
endmodule

module VCVS(1 2 3 4);
parameter G=1
parameter T=0.
vcvs #(.gain(G)) v(2 3 1 4);
endmodule

simulator lang=spice
.options noinsensitive

.subckt CCVS(1 2 3 4);
 .parameter G=1
  V1 1 4 0
  H1 2 3 V1 {G}
.ends

.subckt CCCS(1 2 3 4);
 .parameter G=1
  V1 1 4 0
  F1 2 3 V1 {G}
.ends

.subckt IAC(1 2);
.parameter I=1
* phase missing!
I1 1 2 dc=0 ac={I}
.ends

* Vexp:V1 _net4 _net5 U1="0 V" U2="1 V" T1="0" T2="1 ms" Tr="1  ms" Tf="1 ms"


* Ipulse:I2 _net2 _net3 I1="0" I2="1 A" T1="0" T2="1 ms" Tr="1 ms" Tf="2 ms"
.subckt Ipulse(1 2);
.parameter I1=0
.parameter I2=1
.parameter T2=1m
.parameter TL=1m
.parameter Tr=1n
.parameter Tf=1n
.parameter T1=0

I1 2 1 pulse rise=Tr fall=Tf delay=T1 pv=I2 iv=I1 width={T2-T1-Tr-Tf}
.ends

* Vpulse:V2 _net2 gnd U1="0 V" U2="1 V" T1="5m" T2="10m" Tr="1 ns" Tf="1 ns"
.subckt Vpulse(1 2);
.parameter U1
.parameter U2
.parameter T2
.parameter TL
.parameter Tr
.parameter Tf
.parameter T1

V1 1 2 pulse pv=U2 rise=Tr fall=Tf width={T2-T1-Tf-Tr} iv=U1
.ends

* Vrect:V2 _net1 gnd U="1 V" TH="1 ms" TL="1 ms" Tr="1 ns" Tf="1 ns" Td="0 ns"
.subckt Vrect(1 2);
.parameter U=1
.parameter Td
.parameter Tr
.parameter Tf
.parameter TH
.parameter TL

V1 1 2 pulse pv=U delay=Td rise=Tr fall=Tf period={TH+TL+Tr+Tf} width={TH+Tf+Tr} iv=0
.ends

* Irect:I1 _net0 _net1 I="1" TH="1 ms" TL="1 ms" Tr=".5m" Tf=".5m" Td=".5m"
.subckt Irect(1 2);
.parameter I=1
.parameter TH=1m
.parameter TL=1m
.parameter Tr=1n
.parameter Tf=1n
.parameter Td=0

I1 2 1 pulse rise=Tr fall=Tf delay=Td pv=I iv=0 width={TH-Tr} period={TH+TL}
.ends

* Vac:V1 Gate gnd U="5 V" f="10 MHz" Phase="0" Theta="0"
.subckt Vac(1 2)
.parameter U=1
.parameter f=1
.parameter Phase=0
.parameter Theta=0

*V1 1 2 dc=0 ac={U} tran sin amplitude=U frequency=f delay={(-Phase/3.141592653589793/2.-10)/f}
V1 1 2 dc=0 ac={U} tran sin amplitude=U frequency=f delay={(-Phase/360.-10)/f}
.ends

.subckt Iac(1 2)
.parameter I=1
.parameter f=1
.parameter Phase=0
.parameter Theta=0

I1 2 1 dc=0 ac={I} tran sin amplitude=I frequency=f delay={(-Phase/360.-10)/f}
.ends


* don't need spice for this
* .subckt CCVS(1 2 3 4);
* .parameter G=1
* .R:0 R1 1 4 1n
* H1 2 3 R1 {G}
* .ends
* .subckt CCCS(1 2 3 4);
* .parameter G=1
* .R:0 R1 1 4 1n
* F1 2 3 R1 {G}
* .ends
*

* only admit in dc and tr
.subckt y_dctr 1 2
.parameter y
Y1 1 2 ac {0} dc {y} tran {y}
.ends


.simulator lang=verilog

module Pac(1 2);
parameter Z=50
parameter P=1
parameter f=1
parameter Num=1
parameter Temp

// U is local...
parameter U={sqrt(8 * P * Z)}
Vac #(.U(U) .f(f)) sine(1, i);
y_dctr #(.y({1/Z})) Y1(i 2);
pac_ #(.Num(Num) .Z(Z) .P(P)) sp(1, 2);
endmodule

simulator lang=acs
