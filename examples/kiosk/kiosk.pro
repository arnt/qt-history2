TEMPLATE	= app
CONFIG		= qt warn_on release
TMAKE_CC = g++
#TMAKE_CFLAGS += -O3 -DNDEBUG -DNONANSI_INCLUDES -DGUI_Qt -DQUIET -DGUI_Qt_16BIT
#TMAKE_CXXFLAGS += -O3 -DGUI_Qt -DQUIET  -DGUI_Qt_16BIT
TMAKE_CFLAGS += -O3 -DNDEBUG -DNONANSI_INCLUDES -DGUI_Qt
TMAKE_CXXFLAGS += -O3 -DGUI_Qt -DQUIET
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
                gdith.c gray.c mono.c mainx.c jrevdct.c 16bit.c util32.c\
                ordered.c ordered2.c mb_ordered.c readfile.c \ 
		main.cpp \
		../themes/wood.cpp \
		kioskwidget.cpp \  
		qmpeg.cpp
TARGET		= kiosk
