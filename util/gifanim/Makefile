CXXFLAGS=-I$(QTDIR)/include -Wall -O2
LDFLAGS=-L$(QTDIR)/lib
LDLIBS=-lqt

all: example.gif

example.gif: gifanim example.ini example.pov
	x-povray example.ini
	./gifanim example??.ppm
	mv gif.out example.gif

mockup:
	for i in example??.ppm; \
	do \
	    pnmpaste $$i 6 23 movies.ppm >m$$i; \
	done
	./gifanim mexample??.ppm
	mv gif.out mockup.gif

gifanim: gifanim.o

%.o: %.cpp
	$(COMPILE.cc) $< $(OUTPUT_OPTION)
