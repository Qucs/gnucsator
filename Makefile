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

# LDLIBS =

INSTALL_FILES = \
	lang_qucs.so \
	d_probe.so \
	bm_value.so \
	bm_wrapper.so \
	cmd_wrapper.so

CLEANFILES = $(INSTALL_FILES) *.o *~

all: $(INSTALL_FILES)

lang_qucs.so: l_qucs.h
cmd_wrapper.so: l_qucs.h

%.so : %.cc
	$(CXX) $(CXXFLAGS) $(GNUCAP_CXXFLAGS) $(CPPFLAGS) $(GNUCAP_CPPFLAGS) -o $@ $< $(LDLIBS)

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
