include ../Make2

check:
	@echo not implemented

GC_TESTS = \
	lang_qucs.1.gc

out: $(GC_TESTS:%.gc=%.out)


%.out: %.gc
	LD_LIBRARY_PATH=.. gnucap${SUFFIX} < $< > $@

clean:
	rm -rf *.out

