REQUIRES        = embedded
TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= notepad.h ../compact/keyboard.h ../compact/calibrate.h
SOURCES		= notepad.cpp ../compact/keyboard.cpp ../compact/calibrate.cpp
TARGET		= notepad
