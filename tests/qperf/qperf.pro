TEMPLATE	= app
CONFIG		+= qt warn_on release
!macx:CONFIG += console #why do we set console?
win32-msvc:TMAKE_CXXFLAGS_RELEASE = -O2
HEADERS		= qperf.h
SOURCES		= qperf.cpp	\
		  cache.cpp	\
		  cstring.cpp	\
		  dict.cpp	\
		  object.cpp	\
		  painter.cpp	\
		  pixmap.cpp	\
		  string.cpp	\
		  textstream.cpp
TARGET		= qperf
