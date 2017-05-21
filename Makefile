#
# Makefile for the 'LIVE' Video Disk Recorder plugin
#

# The official name of this plugin.
# This name will be used in the '-P...' option of VDR to load the plugin.
# By default the main source file also carries this name.
PLUGIN := live

### The version number of this plugin (taken from the main source file):
VERSION := $(shell grep '\#define LIVEVERSION ' setup.h | awk '{ print $$3 }' | sed -e 's/[";]//g')

### Check for libpcre c++ wrapper
HAVE_LIBPCRECPP := $(shell pcre-config --libs-cpp)

### The directory environment:
# Use package data if installed...otherwise assume we're under the VDR source directory:
PKGCFG = $(if $(VDRDIR),$(shell pkg-config --variable=$(1) $(VDRDIR)/vdr.pc),$(shell PKG_CONFIG_PATH="$$PKG_CONFIG_PATH:../../.." pkg-config --variable=$(1) vdr))
LIBDIR = $(call PKGCFG,libdir)
LOCDIR = $(call PKGCFG,locdir)
PLGCFG = $(call PKGCFG,plgcfg)
#
TMPDIR ?= /tmp

### The compiler options:
export CFLAGS   = $(call PKGCFG,cflags)
export CXXFLAGS = $(call PKGCFG,cxxflags)

ECPPC ?= ecppc

### The version number of VDR's plugin API:
APIVERSION := $(call PKGCFG,apiversion)

### Allow user defined options to overwrite defaults:
-include $(PLGCFG)

### Determine tntnet and cxxtools versions:
TNTVERSION = $(shell tntnet-config --version | sed -e's/\.//g' | sed -e's/pre.*//g' | awk '/^..$$/ { print $$1."000"} /^...$$/ { print $$1."00"} /^....$$/ { print $$1."0" } /^.....$$/ { print $$1 }')
CXXTOOLVER = $(shell cxxtools-config --version | sed -e's/\.//g' | sed -e's/pre.*//g' | awk '/^..$$/ { print $$1."000"} /^...$$/ { print $$1."00"} /^....$$/ { print $$1."0" } /^.....$$/ { print $$1 }')

CXXFLAGS  += $(shell tntnet-config --cxxflags)
LIBS      += $(shell tntnet-config --libs)

### Optional configuration features
PLUGINFEATURES :=
ifneq ($(HAVE_LIBPCRECPP),)
	PLUGINFEATURES += -DHAVE_LIBPCRECPP
	CXXFLAGS       += $(shell pcre-config --cflags)
	LIBS           += $(HAVE_LIBPCRECPP)
endif

CXXFLAGS += -std=c++11 -Wfatal-errors -Wall # -Werror

### export all vars for sub-makes, using absolute paths
LIBDIR := $(shell cd $(LIBDIR) >/dev/null 2>&1 && pwd)
LOCDIR := $(shell cd $(LOCDIR) >/dev/null 2>&1 && pwd)
export
unexport PLUGIN

### The name of the distribution archive:
ARCHIVE := $(PLUGIN)-$(VERSION)
PACKAGE := vdr-$(ARCHIVE)

### The name of the shared object file:
SOFILE := libvdr-$(PLUGIN).so

### Installed shared object file:
SOINST := $(DESTDIR)$(LIBDIR)/$(SOFILE).$(APIVERSION)

### Includes and Defines (add further entries here):
DEFINES	+= -D_GNU_SOURCE -DPLUGIN_NAME_I18N='"$(PLUGIN)"' -DTNTVERSION=$(TNTVERSION) -DCXXTOOLVER=$(CXXTOOLVER)
VERSIONSUFFIX = gen_version_suffix.h

### The object files (add further files here):
PLUGINOBJS := $(PLUGIN).o thread.o tntconfig.o setup.o i18n.o timers.o \
              tools.o recman.o tasks.o status.o epg_events.o epgsearch.o \
              grab.o md5.o filecache.o livefeatures.o preload.o timerconflict.o \
              users.o osd_status.o
PLUGINSRCS := $(patsubst %.o,%.cpp,$(PLUGINOBJS))

WEB_LIB_PAGES := libpages.a
WEB_DIR_PAGES := pages
WEB_PAGES     := $(WEB_DIR_PAGES)/$(WEB_LIB_PAGES)

WEB_LIB_CSS := libcss.a
WEB_DIR_CSS := css
WEB_CSS     := $(WEB_DIR_CSS)/$(WEB_LIB_CSS)

WEB_LIB_JAVA := libjavascript.a
WEB_DIR_JAVA := javascript
WEB_JAVA     := $(WEB_DIR_JAVA)/$(WEB_LIB_JAVA)

WEBLIBS := $(WEB_PAGES) $(WEB_CSS) $(WEB_JAVA)
SUBDIRS := $(WEB_DIR_PAGES) $(WEB_DIR_CSS) $(WEB_DIR_JAVA)

### The main target:
all: lib i18n

### Implicit rules:
$(WEB_DIR_PAGES)/%.o: $(WEB_DIR_PAGES)/%.cpp $(WEB_DIR_PAGES)/%.ecpp
	$(MAKE) -C $(WEB_DIR_PAGES) PLUGINFEATURES="$(PLUGINFEATURES)" $(notdir $@)

$(WEB_DIR_CSS)/%.o:
	$(MAKE) -C $(WEB_DIR_CSS) PLUGINFEATURES="$(PLUGINFEATURES)" $(notdir $@)

$(WEB_DIR_JAVA)/%.o:
	$(MAKE) -C $(WEB_DIR_JAVA) PLUGINFEATURES="$(PLUGINFEATURES)" $(notdir $@)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(PLUGINFEATURES) $(INCLUDES) $<

### Dependencies:
MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(CXXFLAGS) $(DEFINES) $(PLUGINFEATURES) $(INCLUDES) $(PLUGINSRCS) > $@

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPFILE)
endif

### Internationalization (I18N):
PODIR    := po
I18Npo   := $(wildcard $(PODIR)/*.po)
I18Nmo   := $(addsuffix .mo, $(foreach file, $(I18Npo), $(basename $(file))))
I18Nmsgs := $(addprefix $(DESTDIR)$(LOCDIR)/, $(addsuffix /LC_MESSAGES/vdr-$(PLUGIN).mo, $(notdir $(foreach file, $(I18Npo), $(basename $(file))))))
I18Npot  := $(PODIR)/$(PLUGIN).pot
I18Npot_deps = $(PLUGINSRCS) $(wildcard $(WEB_DIR_PAGES)/*.cpp) setup.h epg_events.h

$(I18Npot): $(I18Npot_deps)
	xgettext -C -cTRANSLATORS --no-wrap --no-location -k -ktr -ktrNOOP --omit-header -o $@ $(I18Npot_deps)

# Need a recursive target here to get I18Npot_deps with the correct list of files (wildcard ...)
.PHONY: make_I18Npot
make_I18Npot:
	$(MAKE) $(I18Npot)

%.mo: %.po
	msgfmt -c -o $@ $<

%.po: $(I18Npot)
	msgmerge -U --no-wrap --no-location --backup=none -q -N $@ $<
	@touch $@

$(I18Nmsgs): $(DESTDIR)$(LOCDIR)/%/LC_MESSAGES/vdr-$(PLUGIN).mo: $(PODIR)/%.mo
	install -D -m644 $< $@

.PHONY: i18n
i18n: subdirs make_I18Npot $(I18Nmo) 

.PHONY: install-i18n
install-i18n: i18n $(I18Nmsgs)

### Targets:

$(VERSIONSUFFIX): FORCE
	./buildutil/version-util $(VERSIONSUFFIX) || ./buildutil/version-util -F $(VERSIONSUFFIX)

.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)

$(SUBDIRS):
ifneq ($(MAKECMDGOALS),clean)
	$(MAKE) -C $@ PLUGINFEATURES="$(PLUGINFEATURES)" all
else
	$(MAKE) -C $@ clean
endif

#$(WEB_PAGES): $(WEB_DIR_PAGES)

#$(WEB_CSS): $(WEB_DIR_CSS)

#$(WEB_JAVA): $(WEB_DIR_JAVA)

$(SOFILE): $(PLUGINOBJS) $(WEBLIBS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared $(PLUGINOBJS) -Wl,--whole-archive $(WEBLIBS) -Wl,--no-whole-archive $(LIBS) -o $@

.PHONY: lib
lib: $(VERSIONSUFFIX) subdirs $(SOFILE)

$(SOINST): $(SOFILE)
	install -D $< $@

.PHONY: install-lib
install-lib: lib $(SOINST)

.PHONY: install
install: install-lib install-i18n

.PHONY: dist
dist: $(I18Npo)
	$(MAKE) clean
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@tar czf $(TMPDIR)/$(PACKAGE).tar.gz -C $(TMPDIR) $(ARCHIVE)
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@echo Distribution package created as $(TMPDIR)/$(PACKAGE).tar.gz

.PHONY: clean
clean: subdirs
	@-rm -f $(I18Nmo) $(I18Npot)
	@-rm -f $(PLUGINOBJS) $(DEPFILE) *.so *.tgz core* *~
	@-rm -f $(VERSIONSUFFIX)

.PRECIOUS: $(I18Npo)

.PHONY: FORCE
FORCE:

