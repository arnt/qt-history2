OBJS=y.tab.o scanner.o y.tab.o project.o main.o makefile.o unixmake.o

vpath %.cpp generators/:generators/win32:generators/unix

CFLAGS=-I. -Igenerators -Igenerators/unix -Igenerators/win32  -I$(QTDIR)/include -g #-DQMAKE_DEBUG
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

scanner.o: scanner.c
	$(CC) -c -o $@ $^

scanner.c: scanner.l
	$(LEX) -o$@ $^

y.tab.c: parser.y
	$(YACC) -d $<

%.d: %.cpp
	@[ -d .deps/ ] || mkdir -p .deps/
	@$(CXX) -M $(CFLAGS) $(<) -o .deps/$(@)

%.d: %.c
	@[ -d .deps/ ] || mkdir -p .deps/
	@$(CC) -M $(CFLAGS) $(<) -o .deps/$(@)


