# Project ID used by some IDEs
GUID 		= {8aad65d9-2941-45be-8c6f-4f20c98d2da5}
TEMPLATE	= app
CONFIG		+= qt warn_on release
HEADERS		=
SOURCES		= main.cpp
TARGET		= test
LIBS		+=  -L$$QT_BUILD_TREE/lib -lqui

