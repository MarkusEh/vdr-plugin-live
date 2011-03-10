# The official name of this plugin.

PLUGIN = live

### Additional options to silence TNTNET warnings
TNTFLAGS   ?= -Wno-overloaded-virtual -Wno-unused-variable
TNTVERSION ?= $(shell tntnet-config --version | sed -e's/\.//g' | sed -e's/pre.*//g' | awk '/^..$$/ { print $$1."000"} /^...$$/ { print $$1."00"} /^....$$/ { print $$1."0" } /^.....$$/ { print $$1 }')

### The C++ compiler and options:

CXX      ?= g++
AR       ?= ar
ECPPC    ?= ecppc

CXXFLAGS ?= -O2 -Woverloaded-virtual -Wall -fPIC
DEFINES  ?= -DPLUGIN_NAME_I18N='"$(PLUGIN)"' -DTNTVERSION=$(TNTVERSION)

### The directory environment:

VDRDIR   ?= ../../../..

### Make sure that necessary options are included:

-include $(VDRDIR)/Make.global

### Allow user defined options to overwrite defaults:

-include $(VDRDIR)/Make.config

### Includes and Defines (add further entries here):

INCLUDES += -I$(VDRDIR)/include -I..

### The object files (add further files here):

OBJS	= menu.o recordings.o schedule.o multischedule.o screenshot.o   \
          timers.o whats_on.o switch_channel.o keypress.o remote.o	\
          channels_widget.o edit_timer.o error.o pageelems.o tooltip.o	\
          vlc.o searchtimers.o edit_searchtimer.o searchresults.o	\
          searchepg.o login.o ibox.o xmlresponse.o play_recording.o	\
          pause_recording.o stop_recording.o ffw_recording.o		\
          rwd_recording.o setup.o content.o epginfo.o timerconflicts.o  \
          recstream.o users.o edit_user.o edit_recording.o

### Default rules:

.PHONY: all clean

all: libpages.a

### Implicit rules:

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(TNTFLAGS) -c $(DEFINES) $(PLUGINFEATURES) $(INCLUDES) $<

%.cpp: %.ecpp
	$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_CPP) $<

# Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile $(OBJS:%.o=%.cpp) $(OBJS:%.o=%.ecpp)
	@$(MAKEDEP) $(DEFINES) $(PLUGINFEATURES) $(INCLUDES) $(OBJS:%.o=%.cpp) > $@

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPFILE)
endif

### Targets:

libpages.a: $(OBJS)
	$(AR) r $@ $^

clean:
	@rm -f *~ *.o core* libpages.a $(OBJS:%.o=%.cpp) $(DEPFILE)
