TEMPLATE	= app
CONFIG		+= qt warn_on release
LIBS		= -lttf -lXxf86dga
HEADERS		= qimagepaintdevice.h
SOURCES		= ipd.cpp \
		  qimagepaintdevice.cpp \
		  qimagepaintdevice_ttf.cpp
TARGET		= ipd
