#
# Makefile for a Video Disk Recorder plugin
#
# $Id: Makefile,v 1.51 2007/10/21 14:26:09 winni Exp $

# The official name of this plugin.
# This name will be used in the '-P...' option of VDR to load the plugin.
# By default the main source file also carries this name.
# IPORTANT: the presence of this macro is important for the Make.config
# file. So it must be defined, even if it is not used here!
#
PLUGIN = live

### The version number of this plugin (taken from the main source file):

VERSION = $(shell grep '\#define LIVEVERSION ' setup.h | awk '{ print $$3 }' | sed -e 's/[";]//g')

### The C++ compiler and options:

CXX	 ?= g++

### tntnet produces some compiler warnings, so we add -Wno-unused-variable -Wno-non-virtual-dtor for nice output ;)
CXXFLAGS ?= -fPIC -O2 -Wall -Woverloaded-virtual -Wno-unused-variable -Wno-non-virtual-dtor
LDFLAGS	 ?= -fPIC -g

ECPPC	 ?= ecppc
CXXFLAGS += `tntnet-config --cxxflags`

LIBS  += $(shell tntnet-config --libs)

### The directory environment:

VDRDIR	 ?= ../../..
LIBDIR	 ?= ../../lib
TMPDIR	 ?= /tmp

### Allow user defined options to overwrite defaults:

-include $(VDRDIR)/Make.config

### The version number of VDR's plugin API (taken from VDR's "config.h"):

APIVERSION = $(shell sed -ne '/define APIVERSION/s/^.*"\(.*\)".*$$/\1/p' $(VDRDIR)/config.h)
I18NTARG   = $(shell if [ `echo $(APIVERSION) | tr [.] [0]` -ge "10507" ]; then echo "i18n"; fi)

### The name of the distribution archive:

ARCHIVE = $(PLUGIN)-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

### Includes and Defines (add further entries here):

INCLUDES += -I$(VDRDIR)/include -Ihttpd

DEFINES	 += -D_GNU_SOURCE -DPLUGIN_NAME_I18N='"$(PLUGIN)"'
export DEFINES

LIBS	 += httpd/libhttpd.a

SUBDIRS	  = httpd pages css javascript

### The object files (add further files here):

PLUGINOBJS = $(PLUGIN).o thread.o tntconfig.o setup.o i18n.o timers.o \
	     tools.o recordings.o tasks.o status.o epg_events.o epgsearch.o \
	     grab.o md5.o filecache.o livefeatures.o preload.o

WEBLIBS	   = pages/libpages.a css/libcss.a javascript/libjavascript.a

### Default rules:

.PHONY: all dist clean SUBDIRS

all: libvdr-$(PLUGIN).so $(I18NTARG)

### Implicit rules:

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $<

# Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(PLUGINOBJS:%.o=%.cpp) > $@

-include $(DEPFILE)

### Internationalization (I18N):

PODIR	  = po
LOCALEDIR = $(VDRDIR)/locale
I18Npo	  = $(wildcard $(PODIR)/*.po)
I18Nmo	  = $(addsuffix .mo, $(foreach file, $(I18Npo), $(basename $(file))))
I18Ndirs  = $(notdir $(foreach file, $(I18Npo), $(basename $(file))))
I18Npot	  = $(PODIR)/$(PLUGIN).pot
I18Nvdrmo = vdr-$(PLUGIN).mo
ifeq ($(strip $(APIVERSION)),1.5.7)
  I18Nvdrmo = $(PLUGIN).mo
endif

%.mo: %.po
	msgfmt -c -o $@ $<

$(I18Npot): PAGES $(PLUGINOBJS:%.o=%.cpp)
	xgettext -C -cTRANSLATORS --no-wrap --no-location -k -ktr -ktrNOOP --msgid-bugs-address='<cwieninger@gmx.de>' -o $@ $(PLUGINOBJS:%.o=%.cpp) pages/*.cpp

$(I18Npo): $(I18Npot)
	msgmerge -U --no-wrap --no-location --backup=none -q $@ $<

i18n: $(I18Nmo)
	@mkdir -p $(LOCALEDIR)
	for i in $(I18Ndirs); do\
	    mkdir -p $(LOCALEDIR)/$$i/LC_MESSAGES;\
	    cp $(PODIR)/$$i.mo $(LOCALEDIR)/$$i/LC_MESSAGES/$(I18Nvdrmo);\
	    done

generate-i18n: i18n-template.h $(I18Npot) $(I18Npo) buildutil/pot2i18n.pl
	buildutil/pot2i18n.pl $(I18Npot) i18n-template.h > i18n-generated.h

### Targets:

SUBDIRS:
	@for dir in $(SUBDIRS); do \
		make -C $$dir CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)" || exit 1; \
	done

PAGES:
	make -C pages CXX="$(CXX)" CXXFLAGS="$(CXXFLAGS)" .dependencies || exit 1;

libvdr-$(PLUGIN).so: SUBDIRS $(PLUGINOBJS)
	$(CXX) $(LDFLAGS) -shared -o $@	 $(PLUGINOBJS) -Wl,--whole-archive $(WEBLIBS) -Wl,--no-whole-archive $(LIBS)
	@cp --remove-destination $@ $(LIBDIR)/$@.$(APIVERSION)

dist: clean
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@tar czf $(PACKAGE).tgz -C $(TMPDIR) $(ARCHIVE)
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@echo Distribution package created as $(PACKAGE).tgz

clean:
	@-rm -f $(PODIR)/*.mo $(PODIR)/*.pot
	@-rm -f $(PLUGINOBJS) $(DEPFILE) *.so *.tgz core* *~
	@for dir in $(SUBDIRS); do \
		make -C $$dir clean ; \
	done

.PRECIOUS: $(I18Npo)
