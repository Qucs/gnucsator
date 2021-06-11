=== A gnucap driver for Qucs

Simulators in future Qucs will be pluggable drivers. This is an example and
under construction.

== requirements

- qucs (refactor+qt5-* branch)
- gnucap (>= 20210000)

== build

$ make

== simulate circuit with gnucap (from cli)

$ qucscli < rc.qx

== simulate circuit with gnucap (from gui)

$ qucs -a ./sim_gnucap -i rc.sch # ... and press F2
