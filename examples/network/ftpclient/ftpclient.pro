REQUIRES         = network full-config
TEMPLATE	 = app
CONFIG		+= qt warn_on release
HEADERS		 =
SOURCES		 = main.cpp
INTERFACES	 = ftpmainwindow.ui \
		   connectdialog.ui
TARGET		 = ftpclient
