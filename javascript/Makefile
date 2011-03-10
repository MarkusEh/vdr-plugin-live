# The official name of this plugin.

PLUGIN = live

### Additional options to silence TNTNET warnings
TNTFLAGS   ?= -Wno-overloaded-virtual -Wno-unused-function
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

OBJS	= treeview.o

### Default rules:

.PHONY: all clean

all: libjavascript.a

### Implicit rules:

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(TNTFLAGS) -c $(DEFINES) $(PLUGINFEATURES) $(INCLUDES) $<

%.cpp: %.js
	$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_JS) -b -m "text/javascript" $<

### Targets:

libjavascript.a: $(OBJS)
	$(AR) r $@ $^

clean:
	@rm -f *~ *.o core* libjavascript.a $(OBJS:%.o=%.cpp)
