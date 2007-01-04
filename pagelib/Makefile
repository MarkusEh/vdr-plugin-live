CXX      ?= g++
AR       ?= ar

CXXFLAGS ?= -O2 -Woverloaded-virtual -Wall -fPIC

INCLUDES += -I.. -I$(VDRDIR)/include

### The directory environment:

VDRDIR   ?= ../../../..

### The object files (add further files here):

OBJS	= sortedtimers.o tools.o

### Default rules:

.PHONY: all clean

all: libpagelib.a

### Implicit rules:

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $<

### Targets:

libpagelib.a: $(OBJS)
	$(AR) r $@ $^

clean:
	@rm -f *~ *.o core* libpagelib.a
