REQUIRES        = workspace full-config
TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= application.h 
SOURCES		= application.cpp \
		  main.cpp 
TARGET		= mdi
DEPENDPATH=../../include

macx-g++ {
	target.path=$${TARGET}.app/Contents/MacOS
	INSTALLS += target
}
