### The official name of this plugin.
PLUGIN := live

### Additional options to silence TNTNET warnings
TNTFLAGS ?= -Wno-overloaded-virtual -Wno-unused-function

### Includes and Defines (add further entries here):
INCLUDES += -I$(VDRDIR)/include -I..

### The object files (add further files here):
OBJS := treeview.o
SRCS := $(patsubst %.o,%.cpp,$(OBJS))

include ../global.mk

### The main target:
all: libjavascript.a
	@true

### Implicit rules:
%.o: %.cpp
	$(call PRETTY_PRINT,"CC javascript/" $@)
	$(Q)$(CXX) $(CXXFLAGS) $(TNTFLAGS) -c $(DEFINES) $(PLUGINFEATURES) $(INCLUDES) $<

ifeq ($(shell test $(TNTVERSION) -ge 30000; echo $$?),0)
%.cpp: %.ecpp
else
%.cpp: %.js
endif
	$(call PRETTY_PRINT,"ECPP javascript/" $@)
	$(Q)$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_JS) -b -m "text/javascript" $<

### Targets:
libjavascript.a: $(OBJS)
	$(call PRETTY_PRINT,"AR javascript/" $@)
	$(Q)$(AR) r $@ $^ $(AR_NUL)

clean:
	$(call PRETTY_PRINT,"CLN javascript/")
	@rm -f *~ *.o core* libjavascript.a $(SRCS)

dist: clean
	@echo "Nothing to do for distribution here ..."

.PRECIOUS: $(SRCS)
