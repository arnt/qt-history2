# Project ID used by some IDEs
GUID 	 = {4acec99e-370a-4ab6-88f2-135269d76b3f}
TEMPLATE = app

CONFIG	+= qt warn_off
LIBS	+= -lqaxcontainer

REQUIRES = shared workspace

SOURCES	 = main.cpp docuwindow.cpp
HEADERS	 = docuwindow.h
FORMS	 = mainwindow.ui invokemethod.ui changeproperties.ui ambientproperties.ui controlinfo.ui
RC_FILE	 = testcon.rc
