

HEADERS += plugin/qfactoryinterface.h plugin/qlibrary.h plugin/quuid.h plugin/qfactoryloader_p.h plugin/qpluginloader.h
SOURCES += plugin/qlibrary.cpp plugin/qfactoryloader.cpp plugin/qpluginloader.cpp plugin/quuid.cpp
win32:SOURCES += plugin/qlibrary_win.cpp
else:SOURCES += plugin/qlibrary_unix.cpp