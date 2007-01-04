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

OBJS	= styles.o

### Default rules:

.PHONY: all clean

all: libcss.a

### Implicit rules:

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $<

%.cpp: %.css
	$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_CSS) -b -m "text/css" $<

### Targets:

libcss.a: $(OBJS)
	$(AR) r $@ $^

clean:
	@rm -f *~ *.o core* libcss.a $(OBJS:%.o=%.cpp)
