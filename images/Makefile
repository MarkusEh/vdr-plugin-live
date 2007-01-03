CXX      ?= g++
AR       ?= ar
ECPPC    ?= ecppc

CXXFLAGS ?= -O2 -Woverloaded-virtual -Wall -fPIC

CXXFLAGS += `tntnet-config --cxxflags`
LDFLAGS  += `tntnet-config --libs`

INCLUDES += -I..

### The directory environment:
VDRDIR   ?= ../../../..

### The object files (add further files here):

OBJS	= logo.o

### Default rules:

.PHONY: all clean

### Implicit rules:

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $<

%.cpp: %.png
	$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_CSS) -b $<

### Targets:

all: libimages.a

libimages.a: $(OBJS)
	$(AR) r $@ $^

clean:
	@rm -f *~ *.o core* libimages.a $(OBJS:%.o=%.cpp)
