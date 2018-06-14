simulator language=verilog
// This File is part of gnucap-qucs
// (C) 2018 Felix Salfelder
// GPLv3

// mapping qucsator names to actual devices
// nonlinear stuff
simulator lang=spice
.options noinsensitive

.subckt Relais(1 2 3 4);
.parameter Vt=.5
.parameter Vh=.1
* WEIRD PORT ORDER (intentional)

.model  sss  sw  ( vt=Vt  vh=Vh  ron=1m  roff=1.E+12 )
S1 1 3 2 4 sss

.ends

.simulator lang=acs

* hide this sckt in listings.
hidemodule Relais
