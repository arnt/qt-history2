	-rm -f ../lib/libqt.so.$(VERSION)
	ld -shared -o ../lib/libqt.so.$(VERSION) \
		$(OBJECTS) $(METAOBJ) -lX11 -lc -lcxx
	-rm -f ../lib/libqt.so
	-ln -s libqt.so.$(VERSION) ../lib/libqt.so
	-rm -f ../lib/libqt.so.1
	-ln -s libqt.so.$(VERSION) ../lib/libqt.so.1

