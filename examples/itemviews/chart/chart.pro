HEADERS     = mainwindow.h \
              pieview.h
SOURCES     = main.cpp \
              mainwindow.cpp \
              pieview.cpp
unix:!mac:LIBS+= -lm
