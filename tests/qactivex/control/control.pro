TEMPLATE    = lib
CONFIG      += qt warn_off staticlib
TARGET      = qaxserver
SOURCES     = qaxserverbase.cpp qaxbindable.cpp qaxfactory.cpp qaxservermain.cpp qaxserverdll.cpp ../shared/types.cpp
HEADERS     = qaxserverbase.h qaxbindable.h qaxfactory.h ../shared/types.h

DESTDIR     = $$QT_BUILD_TREE\lib
DEFINES     += QAX_NODLL