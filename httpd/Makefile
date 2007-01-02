CXX      ?= g++
AR       ?= ar

CXXFLAGS ?= -O2 -Woverloaded-virtual -Wall -fPIC

INCLUDES += -I.

OBJS = dispatcher.o  job.o       regex.o   worker.o \
       listener.o    poller.o    tntnet.o

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

all: libhttpd.a

libhttpd.a: $(OBJS)
	$(AR) r $@ $^

clean:
	rm -f *.o core* libproctools.a proctest $(DEPFILE)
