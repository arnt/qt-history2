TEMPLATE = subdirs
CONFIG += ordered

# this order is important
win32:SUBDIRS += winmain
SUBDIRS += moc core gui sql
contains(QT_CONFIG, opengl): SUBDIRS +=  opengl
!contains(QT_PRODUCT, qt-professional):SUBDIRS += xml network
SUBDIRS += compat
!contains(QT_PRODUCT, qt-professional):SUBDIRS += canvas
SUBDIRS += plugins

embedded:SUBDIRS -= opengl
SUBDIRS += tools

# This gives us a top level debug/release
unix {
     EXTRA_DEBUG_TARGETS =
     EXTRA_RELEASE_TARGETS =
     for(sub, SUBDIRS) {
        !isEqual(sub, moc):!isEqual(sub, plugins) {
            #debug
            eval(debug-$${sub}.depends = $${sub}/$(MAKEFILE) $$EXTRA_DEBUG_TARGETS)
            eval(debug-$${sub}.commands = (cd $$sub && $(MAKE) -f $(MAKEFILE) debug))
            EXTRA_DEBUG_TARGETS += debug-$${sub}
            QMAKE_EXTRA_TARGETS += debug-$${sub}
            #release
            eval(release-$${sub}.depends = $${sub}/$(MAKEFILE) $$EXTRA_RELEASE_TARGETS)
            eval(release-$${sub}.commands = (cd $$sub && $(MAKE) -f $(MAKEFILE) release))
            EXTRA_RELEASE_TARGETS += release-$${sub}
            QMAKE_EXTRA_TARGETS += release-$${sub}
        } else { #both a debug and release target in one
            eval(release-debug-$${sub}.depends = $${sub}/$(MAKEFILE) $$EXTRA_DEBUG_TARGETS)
            eval(release-debug-$${sub}.commands = (cd $$sub && $(MAKE) -f $(MAKEFILE) first))
            EXTRA_RELEASE_TARGETS += release-debug-$${sub}
            EXTRA_DEBUG_TARGETS += release-debug-$${sub}
            QMAKE_EXTRA_TARGETS += release-debug-$${sub}
        }
     }
     debug.depends = $$EXTRA_DEBUG_TARGETS
     release.depends = $$EXTRA_RELEASE_TARGETS
     QMAKE_EXTRA_TARGETS += debug release
}
