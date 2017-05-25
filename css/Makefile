### The official name of this plugin.
PLUGIN := live

### Additional options to silence TNTNET warnings
TNTFLAGS ?= -Wno-overloaded-virtual -Wno-unused-function

### Includes and Defines (add further entries here):
INCLUDES += -I$(VDRDIR)/include -I..

### The object files (add further files here):
OBJS := styles.o
SRCS := $(patsubst %.o,%.cpp,$(OBJS))

include ../global.mk

### The main target:
all: libcss.a
	@true

### Implicit rules:
%.o: %.cpp
	$(call PRETTY_PRINT,"CC css/" $@)
	$(Q)$(CXX) $(CXXFLAGS) $(TNTFLAGS) -c $(DEFINES) $(PLUGINFEATURES) $(INCLUDES) $<

%.cpp: %.css
	$(call PRETTY_PRINT,"ECPP css/" $@)
	$(Q)$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_CSS) -b -m "text/css" $<

### Targets:
libcss.a: $(OBJS)
	$(call PRETTY_PRINT,"AR css/" $@)
	$(Q)$(AR) r $@ $^ $(AR_NUL)

clean:
	$(call PRETTY_PRINT,"CLN css/")
	@rm -f *~ *.o core* libcss.a $(SRCS)

dist: clean
	@echo "Nothing to do for distribution here ..."

.PRECIOUS: $(SRCS)
