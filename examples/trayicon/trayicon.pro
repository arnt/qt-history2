GUID 		= {3736e171-457d-4570-8c96-7a9348118a54}
TEMPLATE	= app
TARGET		= trayicon

CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES	= large-config

HEADERS		= trayicon.h
SOURCES		= main.cpp \
		  trayicon.cpp
INTERFACES	=

win32 {
   SOURCES  	+= trayicon_win.cpp
} else:macx {
   QMAKE_INCREMENTAL = f #sorry, but I cannot use *_mac. if I don't do this.. --Sam
   SOURCES	+= trayicon_mac.cpp
   LIBS		+= -framework Carbon
} else:embedded {
  SOURCES	+=trayicon_qws.cpp
}
