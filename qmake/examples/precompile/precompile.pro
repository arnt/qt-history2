#############################################
#
# Example for using Precompiled Headers
#
#############################################
TEMPLATE  = app
LANGUAGE  = C++
CONFIG	 += console

# Use Precompiled headers (PCH)
PRECOMPH  = stable.h

HEADERS	 += stable.h \
            myobject.h
SOURCES	 += main.cpp \
            myobject.cpp \
            util.cpp
FORMS     = mydialog.ui

