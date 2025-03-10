### The official name of this plugin.
PLUGIN := live

### Additional options to silence TNTNET warnings
TNTFLAGS ?= -Wno-overloaded-virtual -Wno-unused-variable

DEFINES += -DDISABLE_TEMPLATES_COLLIDING_WITH_STL

### Includes and Defines (add further entries here):
INCLUDES += -I$(VDRDIR)/include -I..

### The object files (add further files here):
OBJS := recordings_widget.o play_recording.o timers.o edit_timer.o \
        pageelems.o js.o get_recordings.o delete_recording.o recordings.o \
        epginfo.o schedule.o multischedule.o screenshot.o   \
        whats_on.o switch_channel.o keypress.o remote.o	\
        channels_widget.o error.o tooltip.o	\
        searchtimers.o edit_searchtimer.o searchresults.o	\
        searchepg.o login.o ibox.o xmlresponse.o \
        pause_recording.o stop_recording.o ffw_recording.o		\
        rwd_recording.o setup.o content.o timerconflicts.o  \
        recstream.o users.o edit_user.o edit_recording.o osd.o	\
        playlist.o stream.o stream_data.o menu.o
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
