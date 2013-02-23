#
# Makefile for a Video Disk Recorder plugin
#

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
ECPPC	 ?= ecppc

### This variable is overriden in pages/Makefile because we don't want the
### extra warnings in the tntnet generated files. So if you change here
### something be sure to check pages/Makefile too.
CXXFLAGS ?= -fPIC -O2 -Wall
LDFLAGS	 ?= -fPIC -g

### Check for libpcre c++ wrapper
HAVE_LIBPCRECPP = $(shell pcre-config --libs-cpp)

### The directory environment:

VDRDIR	 ?= ../../..
LIBDIR	 ?= ../../lib
TMPDIR	 ?= /tmp

### Make sure that necessary options are included:

-include $(VDRDIR)/Make.global

### Allow user defined options to overwrite defaults:

-include $(VDRDIR)/Make.config

### The version number of VDR's plugin API (taken from VDR's "config.h"):

APIVERSION = $(shell sed -ne '/define APIVERSION/s/^.*"\(.*\)".*$$/\1/p' $(VDRDIR)/config.h)
I18NTARG   = $(shell if [ `echo $(APIVERSION) | tr [.] [0]` -ge "10507" ]; then echo "i18n"; fi)
TNTVERSION = $(shell tntnet-config --version | sed -e's/\.//g' | sed -e's/pre.*//g' | awk '/^..$$/ { print $$1."000"} /^...$$/ { print $$1."00"} /^....$$/ { print $$1."0" } /^.....$$/ { print $$1 }')
CXXTOOLVER = $(shell cxxtools-config --version | sed -e's/\.//g' | sed -e's/pre.*//g' | awk '/^..$$/ { print $$1."000"} /^...$$/ { print $$1."00"} /^....$$/ { print $$1."0" } /^.....$$/ { print $$1 }')
TNTVERS7   = $(shell ver=$(TNTVERSION); if [ $$ver -ge "1606" ]; then echo "yes"; fi)

CXXFLAGS  += $(shell tntnet-config --cxxflags)
LIBS      += $(shell tntnet-config --libs)

### Optional configuration features
PLUGINFEATURES =
ifneq ($(HAVE_LIBPCRECPP),)
	PLUGINFEATURES += -DHAVE_LIBPCRECPP
	CXXFLAGS       += $(shell pcre-config --cflags)
	LIBS           += $(HAVE_LIBPCRECPP)
endif

### The name of the distribution archive:

ARCHIVE = $(PLUGIN)-$(VERSION)
PACKAGE = vdr-$(ARCHIVE)

### Includes and Defines (add further entries here):

INCLUDES += -I$(VDRDIR)/include
ifneq ($(TNTVERS7),yes)
	INCLUDES += -Ihttpd
	LIBS	 += httpd/libhttpd.a
endif

DEFINES	 += -D_GNU_SOURCE -DPLUGIN_NAME_I18N='"$(PLUGIN)"' -DTNTVERSION=$(TNTVERSION) -DCXXTOOLVER=$(CXXTOOLVER)

SUBDIRS	  = pages css javascript
ifneq ($(TNTVERS7),yes)
	SUBDIRS += httpd
endif

VERSIONSUFFIX = gen_version_suffix.h

### The object files (add further files here):

PLUGINOBJS = $(PLUGIN).o thread.o tntconfig.o setup.o i18n.o timers.o \
	     tools.o recman.o tasks.o status.o epg_events.o epgsearch.o \
	     grab.o md5.o filecache.o livefeatures.o preload.o timerconflict.o \
	     users.o

WEBLIBS	   = pages/libpages.a css/libcss.a javascript/libjavascript.a

### Default rules:

all: libvdr-$(PLUGIN).so $(I18NTARG)

.PHONY: all dist clean subdirs $(SUBDIRS) PAGES

### Implicit rules:

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(PLUGINFEATURES) $(INCLUDES) $<

# Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(PLUGINFEATURES) $(INCLUDES) $(PLUGINOBJS:%.o=%.cpp) > $@

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPFILE)
endif

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
	xgettext -C -cTRANSLATORS --no-wrap --no-location -k -ktr -ktrNOOP --omit-header -o $@ $(PLUGINOBJS:%.o=%.cpp) pages/*.cpp setup.h epg_events.h

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

subdirs: $(SUBDIRS)

$(SUBDIRS):
	@$(MAKE) -C $@ $(MAKECMDGOALS) PLUGINFEATURES="$(PLUGINFEATURES)"

PAGES:
	@$(MAKE) -C pages PLUGINFEATURES="$(PLUGINFEATURES)" .dependencies

$(VERSIONSUFFIX): FORCE
	./buildutil/version-util $(VERSIONSUFFIX) || ./buildutil/version-util -F $(VERSIONSUFFIX)

libvdr-$(PLUGIN).so: $(VERSIONSUFFIX) $(SUBDIRS) $(PLUGINOBJS)
	$(CXX) $(LDFLAGS) -shared -o $@	 $(PLUGINOBJS) -Wl,--whole-archive $(WEBLIBS) -Wl,--no-whole-archive $(LIBS)
	@cp --remove-destination $@ $(LIBDIR)/$@.$(APIVERSION)

ifneq ($(TNTVERS7),yes)
	@echo ""
	@echo "LIVE was built successfully and you can try to use it!"
	@echo ""
	@echo ""
	@echo ""
	@echo ""
	@echo "IMPORTANT INFORMATION:"
	@echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@echo "+ This is one of the *last* CVS versions of LIVE which will   +"
	@echo "+ work with versions of tntnet *less* than 1.6.0.6!           +"
	@echo "+                                                             +"
	@echo "+ This version of LIVE already supports tntnet >= 1.6.0.6.    +"
	@echo "+                                                             +"
	@echo "+ Please upgrade tntnet to at least version 1.6.0.6 soon, if  +"
	@echo "+ you want to keep track of bleeding edge LIVE development.   +"
	@echo "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"
	@echo ""
	@echo ""
	@echo ""
	@echo ""
endif

dist: clean
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@tar czf $(TMPDIR)/$(PACKAGE).tar.gz -C $(TMPDIR) $(ARCHIVE)
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@echo Distribution package created as $(TMPDIR)/$(PACKAGE).tar.gz

clean: $(SUBDIRS)
	@-rm -f $(PODIR)/*.mo $(PODIR)/*.pot
	@-rm -f $(PLUGINOBJS) $(DEPFILE) *.so *.tgz core* *~
	@-rm -f $(VERSIONSUFFIX)

.PRECIOUS: $(I18Npo)

FORCE:
