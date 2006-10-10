COMMON_FOLDER = $$PWD/../common
include(../arthurtester.pri)
CONFIG+=console moc
TEMPLATE = app
INCLUDEPATH += .

# Input
HEADERS += widgets.h interactivewidget.h
SOURCES += interactivewidget.cpp main.cpp 

contains(QT_CONFIG, opengl):QT += opengl

pdf:{
	include(../../pdf/pdf.pri)
	INCLUDEPATH += $$PDFDIR
	DEFINES += QT_PDF_SUPPORT QT_NO_COMPRESS
}

QT += xml svg qt3support
