#the textview/textedit stays in until all internal
#usage of it is removed
HEADERS += \
	../compat/text/qsyntaxhighlighter.h \
	../compat/text/qsyntaxhighlighter_p.h \
	../compat/text/qtextview.h \
	../compat/text/qtextbrowser.h \
	../compat/text/qtextedit.h \
	../compat/text/qmultilineedit.h \
	../compat/text/qrichtext_p.h \
	../compat/text/qsimplerichtext.h \
	../compat/text/qstylesheet.h
SOURCES += \
	../compat/text/qsyntaxhighlighter.cpp \
	../compat/text/qtextview.cpp \
	../compat/text/qtextbrowser.cpp \
	../compat/text/qtextedit.cpp \
	../compat/text/qmultilineedit.cpp \
	../compat/text/qrichtext.cpp \
	../compat/text/qrichtext_p.cpp \
	../compat/text/qsimplerichtext.cpp \
	../compat/text/qstylesheet.cpp


#the menu/menubar stuff stays in until the menu
#combobox and mainwindow
HEADERS += \
        ../compat/widgets/q3menudata.h \
        ../compat/widgets/q3menubar.h \
        ../compat/widgets/q3popupmenu.h
SOURCES += \
        ../compat/widgets/q3menudata.cpp \
        ../compat/widgets/q3menubar.cpp \
        ../compat/widgets/q3popupmenu.cpp
mac:SOURCES += ../compat/widgets/q3menubar_mac.cpp
       
         
