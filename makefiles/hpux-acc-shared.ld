	-rm -f ../lib/libqt.sl
	$(CC) -b -o libqt.sl $(OBJECTS) $(METAOBJ) -lX11
	mv libqt.sl ../lib
