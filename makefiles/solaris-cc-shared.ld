	-rm -f ../lib/libqt.so.$(VERSION)
	$(CC) -G -o ../lib/libqt.so.$(VERSION) \
		$(OBJECTS) $(METAOBJ) -L/usr/openwin/lib -lX11
	-rm -f ../lib/libqt.so
	-ln -s libqt.so.$(VERSION) ../lib/libqt.so
	-rm -f ../lib/libqt.so.1
	-ln -s libqt.so.$(VERSION) ../lib/libqt.so.1

