TEMPLATE        = app
LANGUAGE        = C++
TARGET                = distributor
QT              += compat
CONFIG                += qt warn_on uic3

QTDIR_build:REQUIRES        = "contains(QT_CONFIG, full-config)"

SOURCES                += main.cpp
FORMS                = distributor.ui
