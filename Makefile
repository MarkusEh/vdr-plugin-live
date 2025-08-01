#
# Makefile for the 'LIVE' Video Disk Recorder plugin
#

# The official name of this plugin.
# This name will be used in the '-P...' option of VDR to load the plugin.
# By default the main source file also carries this name.
PLUGIN := live

### The version number of this plugin (taken from the main source file):
HASH := \#
ifeq ($(VERSION),)
  VERSION := $(shell awk '/$(HASH)define LIVEVERSION/ { print $$3 }' setup.h | sed -e 's/[";]//g')
endif

# figure out VERSION_SUFFIX
ifeq ($(VERSION_SUFFIX),)
  ifneq ($(shell which git),)
    ifeq ($(shell test -d .git || echo void),)
      VERS_B := $(shell git branch | grep '^*' | sed -e's/^* //')
      VERS_H := $(shell git show --pretty=format:"%h_%ci" HEAD | head -1 | tr -d ' \-:')
      VERS_P := $(shell git status -uno --porcelain | grep -qc . && echo "_patched")
      VERSION_SUFFIX += _git_$(VERS_B)_$(VERS_H)$(VERS_P)
    endif
  endif
  ifneq ($(shell which quilt),)
    ifeq ($(shell quilt applied 2>&1 > /dev/null; echo $$?),0)
      VERSION_SUFFIX += _quilt_$(shell quilt applied | tr  '\n' '_')
    endif
  endif
# $(info VERSION_SUFFIX = "$(VERSION_SUFFIX)")
endif


PKG_CONFIG ?= pkg-config

### The directory environment:
# Use package data if installed...otherwise assume we're under the VDR source directory:
PKGCFG = $(if $(VDRDIR),$(shell $(PKG_CONFIG) --variable=$(1) $(VDRDIR)/vdr.pc),$(shell PKG_CONFIG_PATH="$$PKG_CONFIG_PATH:../../.." $(PKG_CONFIG) --variable=$(1) vdr))
LIBDIR := $(call PKGCFG,libdir)
LOCDIR := $(call PKGCFG,locdir)
CFGDIR := $(call PKGCFG,configdir)
PLGCFG := $(call PKGCFG,plgcfg)
RESDIR := $(call PKGCFG,resdir)
#
TMPDIR ?= /tmp

### The compiler options:
export CFLAGS   := $(call PKGCFG,cflags)
export CXXFLAGS := $(call PKGCFG,cxxflags)

ECPPC ?= ecppc

### The version number of VDR's plugin API:
APIVERSION := $(call PKGCFG,apiversion)

### Allow user defined options to overwrite defaults:
-include $(PLGCFG)

include global.mk

### Determine tntnet and cxxtools versions:
TNTNET-CONFIG := $(shell which tntnet-config 2>/dev/null)
ifeq ($(TNTNET-CONFIG),)
TNTVERSION := $(shell $(PKG_CONFIG) --modversion tntnet | sed -e's/\.//g' | sed -e's/pre.*//g' | awk '/^..$$/ { print $$1."000"} /^...$$/ { print $$1."00"} /^....$$/ { print $$1."0" } /^.....$$/ { print $$1 }')
CXXFLAGS  += $(shell $(PKG_CONFIG) --cflags tntnet)
LIBS      += $(shell $(PKG_CONFIG) --libs tntnet)
else
TNTVERSION = $(shell tntnet-config --version | sed -e's/\.//g' | sed -e's/pre.*//g' | awk '/^..$$/ { print $$1."000"} /^...$$/ { print $$1."00"} /^....$$/ { print $$1."0" } /^.....$$/ { print $$1 }')
CXXFLAGS  += $(shell tntnet-config --cxxflags)
LIBS      += $(shell tntnet-config --libs)
endif

# $(info $$TNTVERSION is [${TNTVERSION}])

CXXTOOLVER := $(shell cxxtools-config --version | sed -e's/\.//g' | sed -e's/pre.*//g' | awk '/^..$$/ { print $$1."000"} /^...$$/ { print $$1."00"} /^....$$/ { print $$1."0" } /^.....$$/ { print $$1 }')


### Optional configuration features
PLUGINFEATURES :=

# -Wno-deprecated-declarations .. get rid of warning: ‘template<class> class std::auto_ptr’ is deprecated
CXXFLAGS += -std=c++17 -Wfatal-errors -Wundef -Wno-deprecated-declarations

### export all vars for sub-makes, using absolute paths
LIBDIR := $(abspath $(LIBDIR))
LOCDIR := $(abspath $(LOCDIR))
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
DEFINES	+= -DDISABLE_TEMPLATES_COLLIDING_WITH_STL
DEFINES	+= -DVERSION_SUFFIX='"$(VERSION_SUFFIX)"'

### The object files (add further files here):
PLUGINOBJS := $(PLUGIN).o recman.o epg_events.o thread.o tntconfig.o setup.o \
              timers.o tools.o status.o epgsearch.o \
              md5.o filecache.o livefeatures.o preload.o timerconflict.o \
              users.o osd_status.o ffmpeg.o xxhash.o i18n.o
PLUGINSRCS := $(patsubst %.o,%.cpp,$(PLUGINOBJS))

WEB_LIB_PAGES := libpages.a
WEB_DIR_PAGES := pages
WEB_PAGES     := $(WEB_DIR_PAGES)/$(WEB_LIB_PAGES)

WEBLIBS := $(WEB_PAGES)
SUBDIRS := $(WEB_DIR_PAGES)

### The main target:
.PHONY: all
all: version_suffix lib i18n
	@true

### Implicit rules:
$(WEB_DIR_PAGES)/%.o: $(WEB_DIR_PAGES)/%.cpp $(WEB_DIR_PAGES)/%.ecpp
	@$(MAKE) -C $(WEB_DIR_PAGES) --no-print-directory PLUGINFEATURES="$(PLUGINFEATURES)" $(notdir $@)

%.o: %.cpp
	$(call PRETTY_PRINT,"CC" $@)
	$(Q)$(CXX) $(CXXFLAGS) -c $(DEFINES) $(PLUGINFEATURES) $(INCLUDES) $<

### Dependencies:
MAKEDEP := $(CXX) -MM -MG
DEPFILE := .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(CXXFLAGS) $(DEFINES) $(PLUGINFEATURES) $(INCLUDES) $(PLUGINSRCS) > $@

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPFILE)
endif

### For all recursive Targets:

recursive-%:
	@$(MAKE) --no-print-directory VERSION=$(VERSION) VERSION_SUFFIX=$(VERSION_SUFFIX) $*

### Internationalization (I18N):
PODIR    := po
I18Npo   := $(wildcard $(PODIR)/*.po)
I18Nmo   := $(addsuffix .mo, $(foreach file, $(I18Npo), $(basename $(file))))
I18Nmsgs := $(addprefix $(DESTDIR)$(LOCDIR)/, $(addsuffix /LC_MESSAGES/vdr-$(PLUGIN).mo, $(notdir $(foreach file, $(I18Npo), $(basename $(file))))))
I18Npot  := $(PODIR)/$(PLUGIN).pot
I18Npot_deps := $(PLUGINSRCS) $(wildcard $(WEB_DIR_PAGES)/*.cpp) setup.h epg_events.h

$(I18Npot): $(I18Npot_deps)
	$(call PRETTY_PRINT,"GT" $@)
	$(Q)xgettext -C -cTRANSLATORS --no-wrap --no-location -k -ktr -ktrNOOP --omit-header -o $@ $(I18Npot_deps)

.PHONY: I18Nmo
I18Nmo: $(I18Nmo)
	@true

%.mo: %.po
	$(if $(DISABLE_I18Nmo_txt),,@echo "Creating *.mo")
	@msgfmt -c -o $@ $<
	$(eval DISABLE_I18Nmo_txt := 1)

%.po: $(I18Npot)
	$(if $(DISABLE_I18Npo_txt),,@echo "Creating *.po")
	@msgmerge -U --no-wrap --no-location --backup=none -q -N $@ $<
	@touch $@
	$(eval DISABLE_I18Npo_txt := 1)

$(I18Nmsgs): $(DESTDIR)$(LOCDIR)/%/LC_MESSAGES/vdr-$(PLUGIN).mo: $(PODIR)/%.mo
	$(if $(DISABLE_I18Nmoinst_txt),,@echo "Installing *.mo")
	@install -D -m644 $< $@
	$(eval DISABLE_I18Nmoinst_txt := 1)

.PHONY: inst_I18Nmsg
inst_I18Nmsg: $(I18Nmsgs)
	@true

# When building in parallel, this will tell make to keep an order in the steps
recursive-I18Nmo: subdirs
recursive-inst_I18Nmsg: recursive-I18Nmo

.PHONY: i18n
i18n: subdirs recursive-I18Nmo

.PHONY: install-i18n
install-i18n: version_suffix i18n recursive-inst_I18Nmsg

### Targets:

.PHONY: subdirs $(SUBDIRS)
subdirs: $(SUBDIRS)

$(SUBDIRS):
ifneq ($(MAKECMDGOALS),clean)
	@$(MAKE) -C $@ --no-print-directory PLUGINFEATURES="$(PLUGINFEATURES)" all
else
	@$(MAKE) -C $@ --no-print-directory clean
endif

$(SOFILE): $(PLUGINOBJS) $(WEBLIBS)
	$(call PRETTY_PRINT,"LD" $@)
	$(Q)$(CXX) $(CXXFLAGS) $(LDFLAGS) -shared $(PLUGINOBJS) -Wl,--whole-archive $(WEBLIBS) -Wl,--no-whole-archive $(LIBS) -o $@

.PHONY: sofile
sofile: $(SOFILE)
	@true

# When building in parallel, this will tell make to keep an order in the steps
recursive-sofile: subdirs
recursive-soinst: recursive-sofile

.PHONY: lib
lib: subdirs $(PLUGINOBJS) recursive-sofile

.PHONY: soinst
soinst: $(SOINST)

$(SOINST): $(SOFILE)
	$(call PRETTY_PRINT,"Installing" $<)
	$(Q) install -D $< $@

.PHONY: version_suffix
version_suffix:
	@echo "VERSION is $(VERSION)"
	@echo "VERSION_SUFFIX = \"$(VERSION_SUFFIX)\""

.PHONY: install-lib
install-lib: version_suffix lib recursive-soinst

.PHONY: install-web
install-web: version_suffix
	@mkdir -p $(DESTDIR)$(RESDIR)/plugins/$(PLUGIN)
	@cp -a live/* $(DESTDIR)$(RESDIR)/plugins/$(PLUGIN)/

.PHONY: install-conf
install-conf: version_suffix
	mkdir -p $(DESTDIR)$(CFGDIR)/plugins/$(PLUGIN)
	@for i in conf/*; do\
	    if ! [ -e $(DESTDIR)$(CFGDIR)/plugins/$(PLUGIN)/$$i ] ; then\
	        cp -p $$i $(DESTDIR)$(CFGDIR)/plugins/$(PLUGIN);\
	    fi;\
	done

.PHONY: install
install: install-lib install-i18n install-web install-conf

.PHONY: dist
dist: $(I18Npo)
	$(MAKE) --no-print-directory clean
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@mkdir $(TMPDIR)/$(ARCHIVE)
	@cp -a * $(TMPDIR)/$(ARCHIVE)
	@tar czf $(TMPDIR)/$(PACKAGE).tar.gz -C $(TMPDIR) $(ARCHIVE)
	@-rm -rf $(TMPDIR)/$(ARCHIVE)
	@echo Distribution package created as $(TMPDIR)/$(PACKAGE).tar.gz

.PHONY: clean
clean: subdirs
	$(call PRETTY_PRINT,"CLN top")
	@-rm -f $(I18Nmo) $(I18Npot)
	@-rm -f $(PLUGINOBJS) $(DEPFILE) *.so *.tgz core* *~

.PRECIOUS: $(I18Npo)

.PHONY: FORCE
FORCE:

