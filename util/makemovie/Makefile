CXXFLAGS=-I$(QTDIR)/include -Wall -O2
LDFLAGS=-L$(QTDIR)/lib
LDLIBS=-lqt

all: makemovie

makemovie: makemovie.o QwCluster.o

%.o: %.cpp
	$(COMPILE.cc) $< $(OUTPUT_OPTION)
