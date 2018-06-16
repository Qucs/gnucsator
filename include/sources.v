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

simulator lang=spice
.options noinsensitive

.subckt IAC(1 2);
.parameter I=1
* phase missing!
I1 1 2 dc=0 ac={I}
.ends

.subckt VAC(1 2);
.parameter U=1
* phase missing!
V1 1 2 dc=0 ac={I}
.ends

.subckt CCVS(1 2 3 4);
.parameter G=1
.R:0 R1 1 4 1n
H1 2 3 R1 {G}
.ends

.subckt CCCS(1 2 3 4);
.parameter G=1
.R:0 R1 1 4 1n
F1 2 3 R1 {G}
.ends

.simulator lang=acs

* hide this sckt in listings.
hidemodule CCCS
hidemodule CCVS
hidemodule VCCS
hidemodule VCVS
hidemodule IAC
hidemodule VAC
