TEMPLATE = app
CONFIG -= moc

# Input
SOURCES += main.cpp

QCONFIG = core
CONFIG += warn_on console
mac:CONFIG -= resource_fork
