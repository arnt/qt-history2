%:
	mkdir $*
	echo >$*/Makefile "Makefile: $*.pro"
	echo >>$*/Makefile "	tmake $*.pro -o Makefile"
	echo >$*/$*.pro  "TEMPLATE    =	app"
	echo >>$*/$*.pro "CONFIG      =	qt warn_on release"
	echo >>$*/$*.pro "SOURCES	    =	$*.cpp"
	echo >>$*/$*.pro "HEADERS	    =	$*.h"
	echo >>$*/$*.pro "TARGET      =	$*"
	sed -e 's/???/$*/g' skel.cpp >$*/$*.cpp
	sed -e 's/???/$*/g' skel.h >$*/$*.h
	cd $*; make -W $*.pro
