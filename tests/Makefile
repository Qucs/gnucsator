include ../Make2

QUCSATOR = qucsator
GNUCSATOR = ../gnucsator.sh
GNUCAP = gnucap$(SUFFIX)

DIFF = diff -w -u
DIFF = numdiff -r1.e-10 -a1.e-10

SHELL=/bin/bash
LN_S=ln -s

all: gc qucs

COMMON_TESTS = \
	AM_Mod.1.net \
	pac.net \
	sub.1.net \
	lang_qucs.1.net \
	lang_qucs.2.net \
	lang_qucs.3.net \
	MUT.net \
	rc.net \
	sparam_e.net \
	rr.net \
	dio.net \
	Vrect.1.net \
	Vrect.2.net

GC_TESTS = \
	lang_qucs.1.gc \
	c_hide.2.gc \
	IProbe.1.gc \
	d_eqn.1.gc \
	qucs.gc \
	list.1.gc \
	wrap.gc \
	wrap.bad.gc \
	rect.gc

check: diff

diff: $(GC_TESTS:%=%.diff) \
      $(COMMON_TESTS:%=%.gc.diff)

gc: $(GC_TESTS:%.gc=%.gc.out) \
    $(COMMON_TESTS:%.net=%.net.gc.out)

$(COMMON_TESTS:%.net=%.net.gc.out): qucs

qucs:
	[ -e qucs ] || ${LN_S} .. qucs

ENV=LD_LIBRARY_PATH=..${LD_LIBRARY_PATH:%=:%} \
	GNUCAP_PLUGPATH=.:..:${GNUCAP_PKGLIBDIR}${GNUCAP_PLUGPATH:%=:%} \
	QUCSATOR_PKGINCLUDEDIR=../include \
   GNUCAP_SYSCONFDIR=.. \
   PATH=..:$$PATH


%.net.gc.out: %.net
	${ENV} $(GNUCSATOR) -i $< > $@

%.gc.out: %.gc
	${ENV} $(GNUCAP) < $< | grep -v ^@@ | sed -e '1,/^core-lib version/d' \
					                            -e '/default plugins/d' > $@

%.gc.diff: %.gc.out %.gc.ref
	${DIFF} $+

%.net.qucs.out: %.net
	$(QUCSATOR) -i $< > $@

$(COMMON_TESTS:%=%.gc.out) $(GC_TESTS:%=%.out): .P

.PHONY: .P check diff

.P:
	@:

clean:
	rm -rf *.out
