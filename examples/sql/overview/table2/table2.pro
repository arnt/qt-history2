TEMPLATE = app

CONFIG  += qt warn_on release

QTDIR_build:REQUIRES = "contains(QT_CONFIG, full-config)"

HEADERS	 =
SOURCES	 = main.cpp ../connection.cpp

