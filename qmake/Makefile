#qmake code
OBJS=project.o main.o makefile.o unixmake.o borland_bmake.o msvc_nmake.o msvc_dsp.o option.o
vpath %.cpp generators/:generators/win32:generators/unix

#qt code
QTOBJS=qstring.o qtextstream.o qiodevice.o qglobal.o qgdict.o qcstring.o qdatastream.o qgarray.o qbuffer.o qglist.o \
	qcollection.o qfile.o qfile_unix.o qtextcodec.o qregexp.o qgvector.o qgcache.o qbitarray.o qdir.o \
	qfileinfo_unix.o qdir_unix.o qfileinfo.o qdatetime.o qstringlist.o qmap.o
vpath %.cpp $(QTDIR)/src/tools
# LFLAGS=-L$(QTDIR)/lib -lqt

CFLAGS+=-I. -Igenerators -Igenerators/unix -Igenerators/win32  -I$(QTDIR)/include \
	-I$(QTDIR)/src/tools -g -DQT_NO_CODECS
LFLAGS+=

qmake: $(OBJS) $(QTOBJS)
	$(CXX) -o $@ $^ $(LFLAGS)
	ln -s -f $@ $(QTDIR)/bin

DEPDIR:=$(shell [ -d .deps/ ] || mkdir -p .deps/)
-include $(OBJS:.o=.dep)

clean::
	rm -f $(OBJS) $(QTOBJS)

distclean:: clean
	rm -rf qmake .deps/

%.o: %.c
	$(CXX) -c -o $@ $(CFLAGS) $^

%.o: %.cpp
	$(CXX) -c -o $@ $(CFLAGS) $^

%.dep: %.cpp
	@$(CXX) -M $(CFLAGS) $(<) > .deps/$(@)

%.dep: %.c
	@$(CC) -M $(CFLAGS) $(<) > .deps/$(@)
