	-rm -f ../lib/libqt.so.$(VERSION)
	$(CC) -shared -h libqt.so.1 -o libqt.so.$(VERSION) \
		$(OBJECTS) $(METAOBJ) -L/usr/openwin/lib -lX11
	mv libqt.so.$(VERSION) ../lib
	-rm -f ../lib/libqt.so
	-ln -s libqt.so.$(VERSION) ../lib/libqt.so
	-rm -f ../lib/libqt.so.1
	-ln -s libqt.so.$(VERSION) ../lib/libqt.so.1

