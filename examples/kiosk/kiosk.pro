GUID 		= {0fb7cc47-d181-4013-afb2-1783a5ca64e2}
TEMPLATE	= app
TARGET		= kiosk

CONFIG		+= qt warn_on release
TMAKE_CC 	= g++

QTDIR_build:REQUIRES        = embedded

win32-msvc.net {
   TMAKE_CFLAGS += -O2 -DNDEBUG -DNONANSI_INCLUDES
   TMAKE_CXXFLAGS += -O2 -DQUIET
}
else {
   TMAKE_CFLAGS += -O3 -DNDEBUG -DNONANSI_INCLUDES
   TMAKE_CXXFLAGS += -O3 -DQUIET
}

HEADERS		= decoders.h \
		  dither.h \
		  fs2.h \
		  fs4.h \
		  globals.h \
		  proto.h \
		  qmpeg.h \
		  util.h \
		  video.h \
		  ../themes/wood.h \
		  kioskwidget.h
SOURCES		= util.c video.c parseblock.c motionvector.c decoders.c \
                fs2.c fs2fast.c fs4.c hybrid.c hybriderr.c 2x2.c floatdct.c\
                gdith.cpp gray.c mono.c mainx.c jrevdct.c 16bit.c util32.c\
                ordered.c ordered2.c mb_ordered.c readfile.c \
		main.cpp \
		../themes/wood.cpp \
		kioskwidget.cpp \
		qmpeg.cpp
