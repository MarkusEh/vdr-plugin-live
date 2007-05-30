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

OBJS	= logo.o record.o active.o inactive.o button_blue.o	\
	  file.o folder_closed.o folder_open.o plus.o minus.o	\
	  transparent.o edit.o del.o play.o close.o   		\
	  bg_header_h.o bg_header_l.o bg_header_r.o bg_box_h.o	\
	  bg_box_l.o bg_box_r.o movie.o menu_line_bg.o		\
	  bg_line.o bg_tools.o zap.o favicon.o bg_line_top.o	\
	  reload.o stop_update.o one_uparrow.o one_downarrow.o  \
	  tv.o remotecontrol.o search.o record_timer.o		\
	  button_red.o button_green.o button_new.o help.o	\
	  logo_login.o button_yellow.o arrow.o arrow_rec.o	\
	  on_dvd.o pause.o ffw.o rwd.o stop.o imdb.o

### Default rules:

.PHONY: all clean

all: libimages.a

### Implicit rules:

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $(DEFINES) $(INCLUDES) $<

%.cpp: %.png
	$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_CSS) -b -m "image/png" $<

%.cpp: %.gif
	$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_CSS) -b -m "image/gif" $<

%.cpp: %.jpg
	$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_CSS) -b -m "image/jpg" $<

%.cpp: %.ico
	$(ECPPC) $(ECPPFLAGS) $(ECPPFLAGS_CSS) -b -m "image/x-icon" $<

### Targets:

libimages.a: $(OBJS)
	$(AR) r $@ $^

clean:
	@rm -f *~ *.o core* libimages.a $(OBJS:%.o=%.cpp)
