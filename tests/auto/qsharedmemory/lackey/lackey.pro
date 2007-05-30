include(../src/qsharedmemory.pri)

QT = core script

DESTDIR = ./

win32: CONFIG += console
mac:CONFIG -= app_bundle

DEFINES	+= QSHAREDMEMORY_DEBUG
DEFINES	+= QSYSTEMSEMAPHORE_DEBUG

SOURCES		+= main.cpp 
TARGET		= lackey
