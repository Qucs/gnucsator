GNUCAP_CONF = $(shell which gnucap-conf$(SUFFIX))

include Make2

ifneq ($(GNUCAP_CONF),)
    CXX = $(shell $(GNUCAP_CONF) --cxx)
    GNUCAP_CPPFLAGS = $(shell $(GNUCAP_CONF) --cppflags) -DADD_VERSION
    GNUCAP_CXXFLAGS = $(shell $(GNUCAP_CONF) --cxxflags)
	 GNUCAP_LIBDIR   = $(shell $(GNUCAP_CONF) --libdir)
else
    $(info no gnucap-conf, this might not work)
    CXX = g++
    GNUCAP_CXXFLAGS = \
        -g -O0 \
        -Wall -Wextra \
        -Wswitch-enum -Wundef -Wpointer-arith -Woverloaded-virtual \
        -Wcast-qual -Wcast-align -Wpacked -Wshadow -Wconversion \
        -Winit-self -Wmissing-include-dirs -Winvalid-pch \
        -Wvolatile-register-var -Wstack-protector \
        -Wlogical-op -Wvla -Woverlength-strings -Wsign-conversion
    GNUCAP_CPPFLAGS = \
        -DHAVE_LIBREADLINE \
        -DUNIX \
        -DTRACE_UNTESTED
	 GNUCAP_LIBDIR=/usr/share/gnucap
endif

GNUCAP_CXXFLAGS+= -fPIC -shared

QUCS_DEVS = opamp cccs
QUCS_DEVS_SO = $(QUCS_DEVS:%=d_qucs_%.so)

# LDLIBS =

INSTALL_FILES = \
	$(QUCS_DEVS_SO) \
	lang_qucs.so \
	d_probe.so \
	bm_value.so \
	bm_trivial.so \
	bm_wrapper.so \
	cmd_wrapper.so

CLEANFILES = $(INSTALL_FILES) *.o *~

all: $(INSTALL_FILES)

lang_qucs.so: l_qucs.h
cmd_wrapper.so: l_qucs.h

dbg:
	echo $(QUCS_DEVS_SO)

$(QUCS_DEVS_SO): d_qucs_%.so: qucs_wrapper.cc
	$(CXX) -std=c++11 $(CXXFLAGS) $(GNUCAP_CXXFLAGS) -DQUCS_DEVICE=$* \
		$(CPPFLAGS) $(QUCS_CPPFLAGS) $(GNUCAP_CPPFLAGS) -o $@ \
		$< $(QUCS_LDFLAGS) $(QUCS_LIBS) \
	$(LDLIBS)

%.so : %.cc
	$(CXX) $(CXXFLAGS) $(GNUCAP_CXXFLAGS) $(CPPFLAGS) $(GNUCAP_CPPFLAGS) -o $@ $< $(LDLIBS)

QUCS_INCLUDEDIR = $(QUCS_PREFIX)/include
QUCS_CPPFLAGS = -I$(QUCS_INCLUDEDIR) -I$(QUCS_INCLUDEDIR)/qucs-core
QUCS_LDFLAGS = -L$(QUCS_PREFIX)/lib
QUCS_LIBS = -lqucs

install : $(INSTALL_FILES)
	install -d $(GNUCAP_LIBDIR)
	install $(INSTALL_FILES) $(GNUCAP_LIBDIR)

uninstall : 
	(cd $(GNUCAP_LIBDIR) ; rm $(INSTALL_FILES))

clean :
	rm -f $(CLEANFILES)

distclean: clean
	rm Make.override

Make2:
	[ -e $@ ] || echo "# here you may override settings" > $@
