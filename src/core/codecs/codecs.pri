# Qt core library codecs module

!contains(QT_CONFIG, bigcodecs):DEFINES += QT_NO_BIG_CODECS

HEADERS += \
	codecs/qbig5codec.h \
	codecs/qeucjpcodec.h \
	codecs/qeuckrcodec.h \
	codecs/qisciicodec_p.h \
	codecs/qgb18030codec.h \
	codecs/qjiscodec.h \
	codecs/qjpunicode.h \
	codecs/qsjiscodec.h \
	codecs/qtextcodec.h \
	codecs/qtsciicodec.h \
	codecs/qutfcodec.h \
	codecs/qtextcodecinterface_p.h \
	codecs/qtextcodecfactory.h \
	codecs/qtextcodecplugin.h

SOURCES += \
	codecs/qbig5codec.cpp \
	codecs/qeucjpcodec.cpp \
	codecs/qeuckrcodec.cpp \
	codecs/qisciicodec.cpp \
	codecs/qgb18030codec.cpp \
	codecs/qjiscodec.cpp \
	codecs/qjpunicode.cpp \
	codecs/qsjiscodec.cpp \
	codecs/qtextcodec.cpp \
	codecs/qtsciicodec.cpp \
	codecs/qutfcodec.cpp \
	codecs/qtextcodecfactory.cpp \
	codecs/qtextcodecplugin.cpp

unix {
	SOURCES += \
		codecs/qfontcncodec.cpp \
		codecs/qfontjpcodec.cpp \  
		codecs/qfonttwcodec.cpp \
		codecs/qfontkrcodec.cpp \
		codecs/qfonthkcodec.cpp \ 
		codecs/qfontlaocodec.cpp

}
