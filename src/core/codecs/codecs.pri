# Qt core library codecs module

HEADERS += \
	codecs/qisciicodec_p.h \
	codecs/qlatincodec_p.h \
	codecs/qsimplecodec_p.h \
	codecs/qtextcodec.h \
	codecs/qtsciicodec_p.h \
	codecs/qutfcodec_p.h \
	codecs/qtextcodecplugin.h

SOURCES += \
	codecs/qisciicodec.cpp \
	codecs/qlatincodec.cpp \
	codecs/qsimplecodec.cpp \
	codecs/qtextcodec.cpp \
	codecs/qtsciicodec.cpp \
	codecs/qutfcodec.cpp \
	codecs/qtextcodecplugin.cpp

unix {
	SOURCES += codecs/qfontlaocodec.cpp
}
