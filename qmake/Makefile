OBJS=project.o main.o makefile.o unixmake.o borland_bmake.o msvc_nmake.o msvc_dsp.o option.o

vpath %.cpp generators/:generators/win32:generators/unix

CFLAGS=-I. -Igenerators -Igenerators/unix -Igenerators/win32  -I$(QTDIR)/include -g 
LFLAGS=-L$(QTDIR)/lib -lqt -lfl

LEX=flex
YACC=yacc

qmake: $(OBJS)
	$(CXX) -o $@ $^ $(LFLAGS)

-include $(OBJS:.o=.d)

clean::
	rm -f $(OBJS) y.tab.? scanner.c 

distclean:: clean
	rm -rf qmake .deps/

%.o: %.c
	$(CXX) -c -o $@ $(CFLAGS) $^

%.o: %.cpp
	$(CXX) -c -o $@ $(CFLAGS) $^

%.d: %.cpp
	@[ -d .deps/ ] || mkdir -p .deps/
	@$(CXX) -M $(CFLAGS) $(<) -o .deps/$(@)

%.d: %.c
	@[ -d .deps/ ] || mkdir -p .deps/
	@$(CC) -M $(CFLAGS) $(<) -o .deps/$(@)


