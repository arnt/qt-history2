TEMPLATE    =	moc.t
CONFIG	    =   console release qtinc yacc
LEXINPUT    =	moc.l
YACCINPUT   =	moc.y
INCLUDEPATH =	.;../../include
OBJECTS_DIR =	.
MOCGEN	    =	mocgen.cpp
SOURCES     =	$$MOCGEN		\
		../tools/qbuffer.cpp	\
		../tools/qcollect.cpp	\
		../tools/qdatetm.cpp	\
		../tools/qdstream.cpp	\
		../tools/qgarray.cpp	\
		../tools/qgdict.cpp	\
		../tools/qglist.cpp	\
		../tools/qglobal.cpp	\
		../tools/qgvector.cpp	\
		../tools/qiodev.cpp	\
		../tools/qstring.cpp
TARGET	    =	moc
