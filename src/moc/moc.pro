TEMPLATE	= moc.t
CONFIG		= console release qtinc yacc
LEXINPUT	= moc.l
YACCINPUT	= moc.y
INCLUDEPATH	= ../../include .
DEPENDPATH	= ../../include .
OBJECTS_DIR	= .
MOCGEN		= mocgen.cpp
SOURCES		= $$MOCGEN		    \
		  ../tools/qbuffer.cpp	    \
		  ../tools/qcollection.cpp  \
		  ../tools/qcstring.cpp	    \
		  ../tools/qdatetime.cpp    \
		  ../tools/qdatastream.cpp  \
		  ../tools/qgarray.cpp	    \
		  ../tools/qgdict.cpp	    \
		  ../tools/qglist.cpp	    \
		  ../tools/qglobal.cpp	    \
		  ../tools/qgvector.cpp	    \
		  ../tools/qiodevice.cpp    \
		  ../tools/qregexp.cpp	    \
		  ../tools/qstring.cpp
TARGET	    =	moc
