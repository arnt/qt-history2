TEMPLATE	= app
CONFIG		= qt console warn_on release
win32-msvc:TMAKE_CXXFLAGS_RELEASE = -O2
HEADERS		= qperf.h
SOURCES		= qperf.cpp	\
		  cstring.cpp	\
		  dict.cpp	\
		  object.cpp	\
		  painter.cpp	\
		  pixmap.cpp	\
		  string.cpp
TARGET		= qperf
