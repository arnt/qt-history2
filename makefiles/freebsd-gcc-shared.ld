	-rm -f ../lib/libqt.so.$(VERSION)
	ld -Bshareable \
		-o ../lib/libqt.so.$(VERSION) \
		`lorder /usr/lib/c++rt0.o $(OBJECTS) $(METAOBJ) | tsort` \
		-L/usr/X11R6/lib -lX11
	-rm -f ../lib/libqt.so
	-ln -s libqt.so.$(VERSION) ../lib/libqt.so
	-rm -f ../lib/libqt.so.1
	-ln -s libqt.so.$(VERSION) ../lib/libqt.so.1

