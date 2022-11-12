# gnucap-qucs

This package provides a Gnucap based simulation kernel for Qucs, as well as a
component library and related tools.
See [STATUS](STATUS) for the currently available functionality and
{tests,examples}/*.{net,sch} for example circuits.

Your support is gratefully received.

[![donate](https://liberapay.com/assets/widgets/donate.svg "donate through lp")](https://liberapay.com/felixs/donate)

## gnucsator.sh

Emulates qucsator and enhances Qucs (the default simulator in qucs) in some
situations. e.g. by introducing stable/fast dc/tran analysis and extensibility.

## net2verilog.sh

Convert qucsator "net" files into verilog. Embedded commands will be ignored.

## gnucsator

Binary executable. equivalent to gnucap, but loads a different run time
configuration file (gnucsator.rc).

## setup

- install qucs (the gui).
- install gnucap (>=20210107)
  - official gnucap from savannah [1], or
  - apt install gnucap libgnucap-dev # (not yet)
  - note: gnucsator/develop may require gnucap/develop
- install some other dependencies
  - apt install libboost-all-dev # (this is a superset)
  - apt install libgsl-dev
  - apt install libblas-dev
  - apt install numdiff # used in "make check"
- install this package, "./configure; make install"
- type
  $ QUCSATOR=gnucsator.sh qucs
- or try qucs-s (somehow preconfigured to run Qucsator)
  $ QUCSATOR=gnucsator.sh qucs-s
  Remark: The Qucsator executable override can also be specified using the
  Application settings, Simulation->Select default simulator.

## customisation

gnucap-qucs ships the main executable from gnucap-custom as gnucsator, it is
much like the usual gnucap, but loads gnucsator.rc at startup. you may
override this, by setting GNUCAP_SYSTEMSTARTFILE. For example

$ QUCSATOR=gnucsator.sh GNUCAP_SYSTEMSTARTFILE=my_own.rc qucs

will run a qucs session with your own gnucsator rc. make sure this file exists
next to gnucsator.rc (or also set GNUCAP_SYSTEMSTARTPATH..).

## technical notes

hidemodule. The 'list' command is used to convert netlists, it prints
out items in the netlist. This is where module/subckt declarations live.
dynamically loaded modules don't show up in the netlist, but are reachable
through the dispatcher. the hidemodule command moves modules from the netlist
to the dispatcher, so 'list' does not show them.

bm_wrapper. dispatch element+bm combinations through dispatcher. avoids
listing similar to hidemodule & allows probes... (obsolete).

postproc. Qucs requires the data in an upside-down 'dat' format. mangling is
currently implemented for transient output. This is unnecessary and will be
fixed in future qucs. In the meantime, there will be a bypass option
(currently suspended), so other viewers can still be used for displaying.

[1] git://git.sv.gnu.org/gnucap.git
