HEADERS += addtorrentdialog.h \
           bencodeparser.h \
           mainwindow.h \
           metainfo.h \
           peerwireclient.h \
           ratecontroller.h \
           filemanager.h \  
           torrentclient.h \
           trackerclient.h

SOURCES += main.cpp \
           addtorrentdialog.cpp \
           bencodeparser.cpp \
           mainwindow.cpp \
           metainfo.cpp \
           peerwireclient.cpp \
           ratecontroller.cpp \
           filemanager.cpp \
           torrentclient.cpp \
           trackerclient.cpp

# 3rdparty SHA-1 algorithm
SOURCES += 3rdparty/sha1.c
HEADERS += 3rdparty/sha1.h

# Forms and resources
FORMS += forms/addtorrentform.ui
RESOURCES += icons.qrc

QT += network
