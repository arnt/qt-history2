TEMPLATE	= lib
CONFIG		+= qt warn_on release
win32:CONFIG	+= static
SOURCES		= dummy.cpp
TARGET		= qutil
DESTDIR		= $$QMAKE_LIBDIR_QT
VERSION		= 1.0.0
