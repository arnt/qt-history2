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
		  windowmanager.h \
		  linedlg.h \
		  createdlg.h \
		  wizard_skel.h \
		  dlistview.h \
		  dlistviewedit.h \
		  dlistviewedit_skel.h \
		  dmenudlg.h \
		  dmenudlg_skel.h
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
		  windowmanager.cpp \
		  linedlg.cpp \
		  createdlg.cpp \
		  wizard_skel.cpp \
		  dlistview.cpp \
		  dlistviewedit.cpp \
		  dlistviewedit_skel.cpp \
		  dmenudlg.cpp \
		  dmenudlg_skel.cpp
TARGET		= qdialog
TMAKE_CXXFLAGS   += -DQT_FATAL_ASSERT