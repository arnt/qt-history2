	-rm -f ../lib/libqt.sl
	$(CC) -fPIC -shared -o ../lib/libqt.sl \
		$(OBJECTS) $(METAOBJ) -lX11

