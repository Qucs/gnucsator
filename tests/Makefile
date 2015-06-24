include ../Make2

QUCSATOR = qucsator
GNUCSATOR = ../gnucsator.sh
GNUCAP = gnucap$(SUFFIX)

all: gc qucs

COMMON_TESTS = \
	lang_qucs.1.net \
	lang_qucs.2.net \
	lang_qucs.3.net \
	rc.net \
	Vrect.1.net \
	Vrect.2.net

GC_TESTS = \
	IProbe.1.gc \
	d_eqn.1.gc \
	wrap.gc \
	wrap.bad.gc \
	rect.gc

check: $(COMMON_TESTS:%=%.gc.diff) \
       $(GC_TESTS:%=%.diff)

gc: $(GC_TESTS:%.gc=%.gc.out) \
    $(COMMON_TESTS:%.net=%.net.gc.out)

qucs: $(COMMON_TESTS:%.net=%.net.qucs.out)

%.net.gc.out: %.net
	LD_LIBRARY_PATH=.. $(GNUCSATOR) -i $< > $@

%.gc.out: %.gc
	LD_LIBRARY_PATH=.. $(GNUCAP) < $< |& grep -v ^@@ | sed -e '1,/^core-lib version/d' \
					                            -e '/default plugins/d' > $@

%.gc.diff: %.gc.out %.gc.ref
	diff $+

%.net.qucs.out: %.net
	$(QUCSATOR) -i $< > $@

clean:
	rm -rf *.out
