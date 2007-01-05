CXX      ?= g++
AR       ?= ar

CXXFLAGS ?= -O2 -Woverloaded-virtual -Wall -fPIC

CXXFLAGS += `tntnet-config --cxxflags`

### Includes and Defines (add further entries here):

INCLUDES += -I.

### The object files (add further files here):

OBJS = dispatcher.o  job.o       regex.o   worker.o \
       listener.o    poller.o    tntnet.o

### Default rules:

.PHONY: all clean

all: libhttpd.a

### Implicit rules:

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) -o $@ $<

# Dependencies:

MAKEDEP = $(CXX) -MM -MG
DEPFILE = .dependencies
$(DEPFILE): Makefile
	@$(MAKEDEP) $(DEFINES) $(INCLUDES) $(OBJS:%.o=%.cpp) > $@

-include $(DEPFILE)

### Targets:

libhttpd.a: $(OBJS)
	$(AR) r $@ $^

clean:
	@rm -f *.o core* libhttpd.a proctest $(DEPFILE)
