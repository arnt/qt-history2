TEMPLATE = app
LANGUAGE = C++

CONFIG	+= qt warn_on release uic3

REQUIRES = "contains(QT_CONFIG, full-config)"

SOURCES	+= main.cpp
FORMS	 = options.ui
QT	+= compat
