load(qttest_p4)
SOURCES += tst_qimagewriter.cpp
DEFINES += SRCDIR=\\\"$$PWD\\\"
MOC_DIR=tmp
!contains(QT_CONFIG, no-tiff):DEFINES += QTEST_HAVE_TIFF
win32-msvc:QMAKE_CXXFLAGS -= -Zm200
win32-msvc:QMAKE_CXXFLAGS += -Zm800
