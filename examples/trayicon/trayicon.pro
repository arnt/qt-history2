TEMPLATE	= app
TARGET		= trayicon

CONFIG		+= qt warn_on release

QTDIR_build:REQUIRES	= "contains(QT_CONFIG, large-config)"

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
