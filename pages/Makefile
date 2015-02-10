### The official name of this plugin.
PLUGIN = live

### Additional options to silence TNTNET warnings
TNTFLAGS   ?= -Wno-overloaded-virtual -Wno-unused-variable

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

### The main target:
all: libpages.a

### Implicit rules:

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(TNTFLAGS) -c $(DEFINES) $(PLUGINFEATURES) $(INCLUDES) $<

%.cpp: %.ecpp
	$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_CPP) $<

### Dependencies:
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

dist: clean
	@echo "Nothing to do for distribution here ..."
