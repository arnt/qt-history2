load(qttest_p4)

include(../src/qsharedmemory.pri)
CONFIG -= app_bundle

DEFINES	+= QSHAREDMEMORY_DEBUG
DEFINES	+= QSYSTEMSEMAPHORE_DEBUG

#DESTDIR = ../
#SOURCES		+= ../tst_qsharedmemory.cpp 
#TARGET		= tst_qsharedmemory

SOURCES += ../tst_qsharedmemory.cpp
TARGET = ../tst_qsharedmemory

win32 {
  CONFIG(debug, debug|release) {
    TARGET = ../../debug/tst_qsharedmemory
} else {
    TARGET = ../../release/tst_qsharedmemory
  }
}


