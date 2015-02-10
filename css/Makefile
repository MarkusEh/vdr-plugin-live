### The official name of this plugin.
PLUGIN = live

### Additional options to silence TNTNET warnings
TNTFLAGS   ?= -Wno-overloaded-virtual -Wno-unused-function

### Includes and Defines (add further entries here):
INCLUDES += -I$(VDRDIR)/include -I..

### The object files (add further files here):
OBJS	= styles.o

### The main target:
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
