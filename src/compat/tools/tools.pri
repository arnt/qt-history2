# Qt compat module

compat {
	COMPAT_P	= compat

	HEADERS += tools/qasciicache.h \
		  tools/qasciidict.h \
		  tools/q3signal.h \
		  tools/qcstring.h \
		  tools/qdict.h \
		  tools/qgarray.h \
		  tools/qgcache.h \
		  tools/qgdict.h \
		  tools/qglist.h \
		  tools/qgvector.h \
		  tools/qintcache.h \
		  tools/qintdict.h \
		  tools/qmemarray.h \
		  tools/qobjectdict.h \
		  tools/qptrcollection.h \
		  tools/qptrdict.h \
		  tools/qptrlist.h \
		  tools/qptrqueue.h \
		  tools/qptrstack.h \
		  tools/qptrvector.h \
                  tools/qshared.h \
		  tools/qsortedlist.h \
		  tools/qstrlist.h \
		  tools/qstrvec.h \
		  tools/qtl.h \
	          tools/qvaluelist.h \ 
		  tools/qvaluestack.h \
		  tools/qvaluevector.h 

	SOURCES += tools/qcstring.cpp \
		  tools/q3signal.cpp \
		  tools/qgarray.cpp \
		  tools/qgcache.cpp \
		  tools/qgdict.cpp \
		  tools/qglist.cpp \
		  tools/qgvector.cpp \
                  tools/qshared.cpp \
		  tools/qptrcollection.cpp
}
