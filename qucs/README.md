=== A gnucap driver for Qucs

Simulators in future Qucs will be pluggable drivers. This is an example and
under construction.

== requirements

- qucs (modular branch)
- gnucap (>= 20210000)

== build

$ make

== simulate circuit with gnucap (from cli)

(TODO: automate tests)
$ qucscli < rc.qx # some rc circuit simulation
$ qucscli < sim.0.qx # source schematic file
$ qucscli < sim.1.qx # circuit built with verilog, needs gnucap-adms
$ qucscli < switch.0.qx # circuit with a switch
$ qucscli < sub.0.qx # hierarchy test

== simulate circuit with gnucap (from gui)

$ qucs -a ./sim_gnucap -i rc.sch # ... and press F2

or the verilog example...
$ qucs -a plugins/verilog -a ./sim_gnucap -i rc_verilog.sch
