	ar q libqt.a $(OBJECTS) $(METAOBJ)
	ranlib libqt.a
	mv libqt.a ../lib
