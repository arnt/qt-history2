TEMPLATE        = app
TARGET          = sqlbrowser

QT              += sql

HEADERS         = browserwidget.h connectionwidget.h qsqlconnectiondialog.h
SOURCES         = main.cpp browserwidget.cpp connectionwidget.cpp qsqlconnectiondialog.cpp

FORMS           = qsqlconnectiondialog.ui

