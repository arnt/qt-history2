TEMPLATE	= lib
CONFIG		= qt warn_on plugin

HEADERS		= $(QTDIR)/src/codecs/qeucjpcodec.h \
		  $(QTDIR)/src/codecs/qjiscodec.h \
		  $(QTDIR)/src/codecs/qsjiscodec.h \
		  $(QTDIR)/src/codecs/qfontcodecs_p.h \
		  $(QTDIR)/src/codecs/qjpunicode.h

SOURCES		= $(QTDIR)/src/codecs/qeucjpcodec.cpp \
		  $(QTDIR)/src/codecs/qjiscodec.cpp \
		  $(QTDIR)/src/codecs/qsjiscodec.cpp \
		  $(QTDIR)/src/codecs/qfontjpcodec.cpp \
		  $(QTDIR)/src/codecs/qjpunicode.cpp \
		  main.cpp

TARGET		= qjpcodecs
DESTDIR		= $(QTDIR)/plugins/codecs

target.path=$$plugins.path
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/codecs
INSTALLS += target

