
TEMPLATE	 = app

CONFIG		 = qt warn_on release staticlib

HEADERS		 = data.h qinstallationwizard.h

SOURCES		 = main.cpp data.cpp qinstallationwizard.cpp resourcedata.cpp

INTERFACES	+= interface/progressbarwidget.ui interface/installationwizard.ui \
		   interface/welcome.ui interface/license.ui interface/destination.ui \
		   interface/configuration.ui interface/customize.ui \
		   interface/review.ui interface/install.ui


