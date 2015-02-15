include ../Make2

QUCSATOR = qucsator
GNUCSATOR = ../gnucsator.sh

all: gc qucs

check:
	@echo not implemented

COMMON_TESTS = \
	lang_qucs.2.gc

GCONLY_TESTS = \
	lang_qucs.1.gc

GC_TESTS = $(GCONLY_TESTS) $(COMMON_TESTS)

gc: $(GC_TESTS:%.gc=%.gc.out)
qucs: $(COMMON_TESTS:%.gc=%.qucs.out)

%.gc.out: %.gc
	LD_LIBRARY_PATH=.. $(GNUCSATOR) -i $< > $@

%.qucs.out: %.gc
	$(QUCSATOR) -i $< > $@

clean:
	rm -rf *.out *.qucs
