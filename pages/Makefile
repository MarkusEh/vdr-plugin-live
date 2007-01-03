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

OBJS	= menu.o channels.o schedule.o whats_on_now.o

### Default rules:

.PHONY: all clean

### Implicit rules:

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $<

%.cpp: %.ecpp
	$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_CPP) $<

%.cpp: %.gif
	$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_GIF) -b $<

%.cpp: %.jpg
	$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_JPG) -b $<

%.cpp: %.css
	$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_CSS) -b $<

%.cpp: %.js
	$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_JS) -b $<

### Targets:

all: libpages.o

libpages.o: $(OBJS)
	$(LD) -i -o $@ $^

clean:
	@rm -f *~ *.o core* libpages.a $(OBJS:%.o=%.cpp)
