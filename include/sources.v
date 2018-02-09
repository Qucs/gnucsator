simulator language=verilog
// This File is part of gnucap-qucs
// (C) 2018 Felix Salfelder
// GPLv3

// mapping qucsator names to actual devices
// "sources" section

module VCVS(1 2 3 4);
parameter G=1
vcvs #(.gain(G)) v(2 3 1 4);
endmodule

.simulator lang=acs

* hide this sckt in listings.
hidemodule VCVS
