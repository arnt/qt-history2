all: qregexp.ps

qregexp.dvi: qregexp.mp qregexp.tex
	mpost qregexp.mp
	tex qregexp.tex

qregexp.ps: qregexp.dvi
	dvips -o qregexp.ps qregexp.dvi
