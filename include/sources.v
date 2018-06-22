simulator language=verilog
// This File is part of gnucap-qucs
// (C) 2018 Felix Salfelder
// GPLv3

// mapping qucsator names to actual devices
// "sources" section


module VCCS(1 2 3 4);
parameter G=1
vccs #(.gm(G)) v(2 3 1 4);
endmodule

module VCVS(1 2 3 4);
parameter G=1
vcvs #(.gain(G)) v(2 3 1 4);
endmodule

module CCVS(1, 2, 3, 4);
parameter G=1
vsource #(.dc(0)) p(1, 4);
// looks like a hack. maybe it is.
ccvs #(.gain(G)) HH(2, 3, p);
endmodule

module CCCS(1 2 3 4);
parameter G=1
vsource #(.dc(0)) p(1, 4);
// looks like a hack. maybe it is.
cccs #(.gain(G)) FF(2, 3, p)
endmodule

simulator lang=spice
.options noinsensitive

.subckt IAC(1 2);
.parameter I=1
* phase missing!
I1 1 2 dc=0 ac={I}
.ends

* Vexp:V1 _net4 _net5 U1="0 V" U2="1 V" T1="0" T2="1 ms" Tr="1  ms" Tf="1 ms"


* Ipulse:I2 _net2 _net3 I1="0" I2="1 A" T1="0" T2="1 ms" Tr="1 ms" Tf="2 ms"
.subckt Ipulse(1 2);
.parameter I=1
.parameter T2=1m
.parameter TL=1m
.parameter Tr=1n
.parameter Tf=1n
.parameter T1=0

I1 2 1 pulse rise=Tr fall=Tf delay=T1 pv=I iv=0 width={T2-T1-Tr-Tf}
.ends
.hidemodule Ipulse

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
.hidemodule Irect

.subckt VAC(1 2);
.parameter U=1
* phase missing!
V1 1 2 dc=0 ac={I}
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

.simulator lang=acs

* hide this sckt in listings.
hidemodule CCCS
hidemodule CCVS
hidemodule VCCS
hidemodule VCVS
hidemodule IAC
hidemodule VAC
