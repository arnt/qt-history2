%:
	mkdir $*
	echo >$*/Makefile "Makefile: $*.pro"
	echo >>$*/Makefile "	tmake $*.pro -o Makefile"
	echo >$*/$*.pro  "TEMPLATE    =	app"
	echo >>$*/$*.pro "CONFIG      =	qt warn_on release"
	echo >>$*/$*.pro "SOURCES	    =	$*.cpp"
	echo >>$*/$*.pro "TARGET      =	$*"
	touch $*/$*.cpp
	cd $*; make -W $*.pro
