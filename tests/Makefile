include ../Make2

QUCSATOR = qucsator
GNUCAP = gnucap${SUFFIX}

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
	LD_LIBRARY_PATH=.. $(GNUCAP) -a lang_qucs.so -i qucs.gc < $< > $@

%.qucs.out: %.gc
	$(QUCSATOR) -i $< > $@

clean:
	rm -rf *.out *.qucs
