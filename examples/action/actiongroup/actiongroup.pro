TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		= editor.h
SOURCES		= tiny_editor.cpp \
                  editor.cpp 
TARGET		= tiny_editor
DEPENDPATH=../../../include
REQUIRES=full-config
