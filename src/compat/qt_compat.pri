# Qt compat module

compat {
	COMPAT_P	= compat
	HEADERS += $$COMPAT_H/qasciicache.h \
		  $$COMPAT_H/qasciidict.h \
		  $$COMPAT_H/q3signal.h \
		  $$COMPAT_H/qcstring.h \
		  $$COMPAT_H/qdict.h \
		  $$COMPAT_H/qgarray.h \
		  $$COMPAT_H/qgcache.h \
		  $$COMPAT_H/qgdict.h \
		  $$COMPAT_H/qglist.h \
		  $$COMPAT_H/qgvector.h \
		  $$COMPAT_H/qintcache.h \
		  $$COMPAT_H/qintdict.h \
		  $$COMPAT_H/qmemarray.h \
		  $$COMPAT_H/qptrcollection.h \
		  $$COMPAT_H/qptrdict.h \
		  $$COMPAT_H/qptrlist.h \
		  $$COMPAT_H/qptrqueue.h \
		  $$COMPAT_H/qptrstack.h \
		  $$COMPAT_H/qptrvector.h \
		  $$COMPAT_H/qsortedlist.h \
		  $$COMPAT_H/qstrlist.h \
		  $$COMPAT_H/qstrvec.h \
	          $$COMPAT_H/qvaluelist.h \ 
		  $$COMPAT_H/qvaluestack.h \
		  $$COMPAT_H/qvaluevector.h

	SOURCES += $$COMPAT_CPP/qcstring.cpp \
		  $$COMPAT_CPP/q3signal.cpp \
		  $$COMPAT_CPP/qgarray.cpp \
		  $$COMPAT_CPP/qgcache.cpp \
		  $$COMPAT_CPP/qgdict.cpp \
		  $$COMPAT_CPP/qglist.cpp \
		  $$COMPAT_CPP/qgvector.cpp \
		  $$COMPAT_CPP/qptrcollection.cpp
}
