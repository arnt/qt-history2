	-rm -f ../lib/libqt.a
	/usr/lpp/xlC/bin/makeC++SharedLib -p 0 -o libqt.so.$(VERSION) -lX11 \
		$(OBJECTS) $(METAOBJ)
	ar q libqt.a libqt.so.$(VERSION)
	ranlib libqt.a
	mv libqt.a ../lib
