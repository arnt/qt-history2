	-rm -f ../lib/libqt.sl
	$(CC) -fPIC -shared -o libqt.sl \
		$(OBJECTS) $(METAOBJ) -lX11
	mv libqt.sl ../lib
