%:
	mkdir $*
	sed -e 's/???/$*/g' skel.cpp >$*/$*.cpp
	sed -e 's/???/$*/g' skel.h >$*/$*.h
	cd $*; makeqtmake $*
