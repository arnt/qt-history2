OBJS=project.o main.o makefile.o unixmake.o borland_bmake.o msvc_nmake.o msvc_dsp.o option.o

vpath %.cpp generators/:generators/win32:generators/unix

CFLAGS=-I. -Igenerators -Igenerators/unix -Igenerators/win32  -I$(QTDIR)/include -I$(QTDIR)/src/tools -g 
LFLAGS=-L$(QTDIR)/lib -lqt

qmake: $(OBJS)
	$(CXX) -o $@ $^ $(LFLAGS)

-include $(OBJS:.o=.d)

clean::
	rm -f $(OBJS)

distclean:: clean
	rm -rf qmake .deps/

%.o: %.c
	$(CXX) -c -o $@ $(CFLAGS) $^

%.o: %.cpp
	$(CXX) -c -o $@ $(CFLAGS) $^

%.d: %.cpp
	@[ -d .deps/ ] || mkdir -p .deps/
	@$(CXX) -M $(CFLAGS) $(<) > .deps/$(@)

%.d: %.c
	@[ -d .deps/ ] || mkdir -p .deps/
	@$(CC) -M $(CFLAGS) $(<) > .deps/$(@)
