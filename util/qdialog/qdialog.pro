TEMPLATE	= app
CONFIG		= qt warn_on debug
HEADERS		= mainwindow.h \
		  inspector.h \
		  widgetsbar.h \
		  formeditor.h \	
		  widgetinfo.h \
		  gridopt.h \
		  dlayout.h \
		  qform.h \
		  qtable.h \
		  xml.h \
		  connectdlg.h \
		  windowmanager.h
SOURCES		= mainwindow.cpp \
		  inspector.cpp \
		  widgetsbar.cpp \
		  formeditor.cpp \
		  main.cpp \
		  widgetinfo.cpp \
		  gridopt.cpp \
		  qform.cpp \
		  dlayout.cpp \
		  xml.cpp \
		  connectdlg.cpp \
		  windowmanager.cpp
TARGET		= qdialog
TMAKE_CXXFLAGS   += -DQT_FATAL_ASSERT