	-rm -f ../lib/libqt.sl
	$(CC) -b -o ../lib/libqt.sl $(OBJECTS) $(METAOBJ) -lX11

