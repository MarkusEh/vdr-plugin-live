CXX      ?= g++
AR       ?= ar
ECPPC    ?= ecppc

CXXFLAGS ?= -O2 -Woverloaded-virtual -Wall -fPIC

CXXFLAGS += `tntnet-config --cxxflags`
LDFLAGS  += `tntnet-config --libs`

INCLUDES += -I.. -I$(VDRDIR)/include

### The directory environment:

VDRDIR   ?= ../../../..

### The object files (add further files here):

OBJS	= menu.o channels.o recordings.o schedule.o \
          screenshot.o timers.o whats_on.o \
          keypress.o remote.o channels_widget.o edit_timer.o       \
          error.o pageelems.o tooltip_widget.o

### Default rules:

.PHONY: all clean

all: libpages.a

### Implicit rules:

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $<

%.cpp: %.ecpp
	$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_CPP) $<

### Targets:

libpages.a: $(OBJS)
	$(AR) r $@ $^

clean:
	@rm -f *~ *.o core* libpages.a $(OBJS:%.o=%.cpp)
