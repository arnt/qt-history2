	ar q libqt.a `lorder $(OBJECTS) $(METAOBJ) | tsort`
	mv libqt.a ../lib

