TEMPLATE	= lib
CONFIG		= qt warn_on release
WIN32:CONFIG   += dll
HEADERS		=
SOURCES		= main.cpp \
		  ../../../moc_qapplicationinterfaces.cpp

INTERFACES	=
DESTDIR		= ../../
