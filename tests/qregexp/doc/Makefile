all: qregexp.ps

qregexp.dvi: qregexp.mp qregexp.tex
	p4 edit qregexp.1 qregexp.2 qregexp.3 qregexp.dvi
	mpost qregexp.mp
	tex qregexp.tex

qregexp.ps: qregexp.dvi
	p4 edit qregexp.ps
	dvips -o qregexp.ps qregexp.dvi

clean:
	rm -f qregexp.log qregexp.mpx
