# Qt accessibility module

accessibility {
      HEADERS += $$ACCESSIBLE_H/qaccessible.h \
		 $$ACCESSIBLE_H/qaccessibleobject.h \
		 $$ACCESSIBLE_H/qaccessiblewidget.h
      SOURCES += $$ACCESSIBLE_CPP/qaccessible.cpp \
		 $$ACCESSIBLE_CPP/qaccessibleobject.cpp \
		 $$ACCESSIBLE_CPP/qaccessiblewidget.cpp

      mac:SOURCES += $$ACCESSIBLE_CPP/qaccessible_mac.cpp
      else:win32:SOURCES += $$ACCESSIBLE_CPP/qaccessible_win.cpp
      else:SOURCES += $$ACCESSIBLE_CPP/qaccessible_unix.cpp
}
