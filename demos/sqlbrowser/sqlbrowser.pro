TEMPLATE        = app
TARGET          = sqlbrowser

QT              = core gui sql

CONFIG         += debug

HEADERS         = browserwidget.h connectionwidget.h
SOURCES         = main.cpp browserwidget.cpp connectionwidget.cpp

