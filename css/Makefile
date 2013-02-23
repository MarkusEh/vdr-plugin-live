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

OBJS	= styles.o

### Default rules:

.PHONY: all clean

all: libcss.a

### Implicit rules:

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(TNTFLAGS) -c $(DEFINES) $(PLUGINFEATURES) $(INCLUDES) $<

%.cpp: %.css
	$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_CSS) -b -m "text/css" $<

### Targets:

libcss.a: $(OBJS)
	$(AR) r $@ $^

clean:
	@rm -f *~ *.o core* libcss.a $(OBJS:%.o=%.cpp)

dist: clean
	@echo "Nothing to do for distribution here ..."
