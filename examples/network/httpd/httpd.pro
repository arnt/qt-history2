REQUIRES        = network large-config
TEMPLATE	= app
CONFIG		+= qt warn_on release
TMAKE_CXXFLAGS  += -I../../src
HEADERS		= 
SOURCES		= httpd.cpp
TARGET		= httpd
