load(qttest_p4)
SOURCES += tst_bic.cpp qbic.cpp
QT = core

QMAKE_RESOURCE_FLAGS *= -compress 9

# resource too big for win32-msvc
!win32-msvc:!win32-msvc.net:RESOURCES += bic.qrc
