# Qt core library codecs module

HEADERS += \
	codecs/qbig5codec_p.h \
	codecs/qeucjpcodec_p.h \
	codecs/qeuckrcodec_p.h \
	codecs/qisciicodec_p.h \
	codecs/qgb18030codec_p.h \
	codecs/qjiscodec_p.h \
	codecs/qjpunicode.h \
	codecs/qlatincodec_p.h \
	codecs/qsjiscodec_p.h \
	codecs/qsimplecodec_p.h \
	codecs/qtextcodec.h \
	codecs/qtsciicodec_p.h \
	codecs/qutfcodec_p.h \
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
	codecs/qlatincodec.cpp \
	codecs/qsjiscodec.cpp \
	codecs/qsimplecodec.cpp \
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
