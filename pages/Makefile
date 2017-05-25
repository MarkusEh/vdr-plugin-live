### The official name of this plugin.
PLUGIN := live

### Additional options to silence TNTNET warnings
TNTFLAGS ?= -Wno-overloaded-virtual -Wno-unused-variable

### Includes and Defines (add further entries here):
INCLUDES += -I$(VDRDIR)/include -I..

### The object files (add further files here):
OBJS := menu.o recordings.o schedule.o multischedule.o screenshot.o   \
        timers.o whats_on.o switch_channel.o keypress.o remote.o	\
        channels_widget.o edit_timer.o error.o pageelems.o tooltip.o	\
        vlc.o searchtimers.o edit_searchtimer.o searchresults.o	\
        searchepg.o login.o ibox.o xmlresponse.o play_recording.o	\
        pause_recording.o stop_recording.o ffw_recording.o		\
        rwd_recording.o setup.o content.o epginfo.o timerconflicts.o  \
        recstream.o users.o edit_user.o edit_recording.o osd.o
SRCS := $(patsubst %.o,%.cpp,$(OBJS))
ESRCS := $(patsubst %.o,%.ecpp,$(OBJS))
ESRCS_DEPS := $(patsubst %.o,.%.edep,$(OBJS))

include ../global.mk

### The main target:
all: libpages.a
	@true

### Implicit rules:

%.o: %.cpp
	$(call PRETTY_PRINT,"CC pages/" $@)
	$(Q)$(CXX) $(CXXFLAGS) $(TNTFLAGS) -c $(DEFINES) $(PLUGINFEATURES) $(INCLUDES) $<

%.cpp: %.ecpp
	$(call PRETTY_PRINT,"ECPP pages/" $@)
	$(Q)$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_CPP) $<

.%.edep: %.ecpp
	@$(ECPPC) -M $(ECPPFLAGS) $(ECPPFLAGS_CPP) $< > $@

### Dependencies:
MAKEDEP := $(CXX) -MM -MG
DEPFILE := .dependencies
$(DEPFILE): Makefile $(SRCS) $(ESRCS)
	@$(MAKEDEP) $(DEFINES) $(PLUGINFEATURES) $(INCLUDES) $(SRCS) > $@

ifneq ($(MAKECMDGOALS),clean)
-include $(DEPFILE)
-include $(ESRCS_DEPS)
endif

### Targets:
libpages.a: $(OBJS)
	$(call PRETTY_PRINT,"AR pages/" $@)
	$(Q)$(AR) r $@ $^ $(AR_NUL)

clean:
	$(call PRETTY_PRINT,"CLN pages/")
	@rm -f *~ *.o core* libpages.a $(SRCS) $(DEPFILE) $(ESRCS_DEPS)

dist: clean
	@echo "Nothing to do for distribution here ..."

.PRECIOUS: $(SRCS)
