# Makefile for Qucs plugin
# (c) 2020 Felix Salfelder
# GPLv3+

GNUCAP_CONF = $(shell which gnucap-conf)
QUCS_CONF = $(shell which qucs-conf)

ifneq ($(GNUCAP_CONF),)
 $(info asking $(GNUCAP_CONF))
 # CXX             = $(shell $(GNUCAP_CONF) --cxx)   # c++
 GNUCAP_CPPFLAGS = $(shell $(GNUCAP_CONF) --cppflags) -DADD_VERSION
 GNUCAP_LDFLAGS  = $(shell $(GNUCAP_CONF) --ldflags)
 GNUCAP_CXXFLAGS = $(shell $(GNUCAP_CONF) --cxxflags)
else
 $(error no gnucap-conf, this will not work)
endif

ifneq ($(QUCS_CONF),)
 $(info asking $(QUCS_CONF))
 # TODO. complete the move to configure.
 QUCS_CXX      = $(shell $(QUCS_CONF) --cxx)   # c++
 QUCS_CPPFLAGS = $(shell $(QUCS_CONF) --cppflags) -DADD_VERSION
 QT_CPPFLAGS = $(shell $(QUCS_CONF) --qt-cppflags)
 QUCS_LDFLAGS  = $(shell $(QUCS_CONF) --ldflags)
 QUCS_CXXFLAGS = $(shell $(QUCS_CONF) --cxxflags)
 HAVE_QUCS=yes
else
 HAVE_QUCS=no
 $(error no qucs-conf, this will not work)
endif

include MakeList

LIBS = -lgnucap # -lboost_thread

all: ${TARGET}

MY_CPPFLAGS = ${GNUCAP_CPPFLAGS}

MY_CPPFLAGS += ${QUCS_CPPFLAGS}

# BUG: we don't need or want Qt here, but it is entangled with the qucs headers still.
MY_CPPFLAGS += ${QT_CPPFLAGS}

RAW_O = ${RAW_SRCS:%.cc=%.o}

${TARGET}: ${RAW_O}
	${QUCS_CXX} -shared -o $@ ${RAW_O} ${LIBS}

%.o: %.cc
	${QUCS_CXX} -c -fPIC ${MY_CPPFLAGS} ${CPPFLAGS} ${QUCS_CXXFLAGS} ${CXXFLAGS} $< -o $@

