CXX      ?= g++
AR       ?= ar
ECPPC    ?= ecppc

INCLUDES += -I.. -I$(VDRDIR)/include

### The directory environment:

VDRDIR   ?= ../../../..

### The object files (add further files here):

OBJS	= menu.o recordings.o schedule.o screenshot.o timers.o		\
          whats_on.o switch_channel.o keypress.o remote.o		\
          channels_widget.o edit_timer.o error.o pageelems.o tooltip.o	\
          vlc.o searchtimers.o edit_searchtimer.o searchresults.o	\
          searchepg.o login.o ibox.o xmlresponse.o play_recording.o	\
          pause_recording.o stop_recording.o ffw_recording.o		\
          rwd_recording.o setup.o content.o epginfo.o timerconflicts.o

### Default rules:

.PHONY: all clean

all: libpages.a

### Implicit rules:

### tntnet produces some compiler warnings,
### so we add -Wno-unused-variable -Wno-non-virtual-dtor for nice output ;)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -Wno-unused-variable -Wno-non-virtual-dtor -c $(DEFINES) $(INCLUDES) $<

%.cpp: %.ecpp
	$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_CPP) $<

# Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile $(OBJS:%.o=%.cpp) $(OBJS:%.o=%.ecpp)
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.cpp) > $@

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPFILE)
endif

### Targets:

libpages.a: $(OBJS)
	$(AR) r $@ $^

clean:
	@rm -f *~ *.o core* libpages.a $(OBJS:%.o=%.cpp) $(DEPFILE)
