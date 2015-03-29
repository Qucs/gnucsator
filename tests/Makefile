include ../Make2

QUCSATOR = qucsator
GNUCSATOR = ../gnucsator.sh
GNUCAP = gnucap$(SUFFIX)

all: gc qucs

check:
	@echo not implemented

COMMON_TESTS = \
	lang_qucs.1.net \
	lang_qucs.2.net \
	lang_qucs.3.net

GC_TESTS = \
	wrap.gc

gc: $(GC_TESTS:%.gc=%.gc.out) \
    $(COMMON_TESTS:%.net=%.net.gc.out)

qucs: $(COMMON_TESTS:%.net=%.net.qucs.out)

%.net.gc.out: %.net
	LD_LIBRARY_PATH=.. $(GNUCSATOR) -i $< > $@

%.gc.out: %.gc
	LD_LIBRARY_PATH=.. $(GNUCAP) < $< > $@

%.net.qucs.out: %.net
	$(QUCSATOR) -i $< > $@

clean:
	rm -rf *.out
