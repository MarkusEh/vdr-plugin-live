### The official name of this plugin.
PLUGIN = live

### Additional options to silence TNTNET warnings
TNTFLAGS   ?= -Wno-overloaded-virtual -Wno-unused-function

### Includes and Defines (add further entries here):
INCLUDES += -I$(VDRDIR)/include -I..

### The object files (add further files here):
OBJS	= treeview.o

### The main target:
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

dist: clean
	@echo "Nothing to do for distribution here ..."
