	-rm -f ../lib/libqt.so.$(VERSION)
	$(CC) -shared -Wl,-soname,libqt.so.1 \
		-o ../lib/libqt.so.$(VERSION) \
		$(OBJECTS) $(METAOBJ) -L/usr/X11R6/lib -lX11
	-rm -f ../lib/libqt.so
	-ln -s libqt.so.$(VERSION) ../lib/libqt.so
	-rm -f ../lib/libqt.so.1
	-ln -s libqt.so.$(VERSION) ../lib/libqt.so.1

