TEMPLATE	= lib
CONFIG		+= qt warn_on plugin
REQUIRES	= !bigcodecs

HEADERS		= $(QTDIR)/src/codecs/qfontcodecs_p.h

SOURCES		= $(QTDIR)/src/codecs/qfontarcodec.cpp \
		  main.cpp

TARGET		= qarcodecs
DESTDIR		= ../../../../plugins/codecs

target.path=$$plugins.path/codecs
isEmpty(target.path):target.path=$$QT_PREFIX/plugins/codecs
INSTALLS += target
