TEMPLATE	= lib
CONFIG		+= qt warn_on plugin
REQUIRES	= !bigcodecs

HEADERS		= $(QTDIR)/src/codecs/qeucjpcodec.h \
		  $(QTDIR)/src/codecs/qjiscodec.h \
		  $(QTDIR)/src/codecs/qsjiscodec.h \
		  $(QTDIR)/src/codecs/qjpunicode.h \
		  $(QTDIR)/src/codecs/qfontcodecs_p.h

SOURCES		= $(QTDIR)/src/codecs/qeucjpcodec.cpp \
		  $(QTDIR)/src/codecs/qjiscodec.cpp \
		  $(QTDIR)/src/codecs/qsjiscodec.cpp \
		  $(QTDIR)/src/codecs/qjpunicode.cpp \
		  $(QTDIR)/src/codecs/qfontjpcodec.cpp \
		  main.cpp

TARGET		= qjpcodecs
DESTDIR		= ../../../../plugins/codecs

target.path=$$plugins.path/codecs
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/codecs
INSTALLS += target

