TEMPLATE = subdirs
CONFIG += ordered

# this order is important
win32:SUBDIRS += winmain
SUBDIRS += tools/moc tools/rcc corelib xml gui sql network
!embedded:contains(QT_CONFIG, opengl): SUBDIRS += opengl
contains(QT_CONFIG, qt3support): SUBDIRS += qt3support 
!cross_compile {
    SUBDIRS += tools/uic 
    contains(QT_CONFIG, qt3support): SUBDIRS += tools/uic3
}
SUBDIRS += plugins

# This gives us a top level debug/release
EXTRA_DEBUG_TARGETS =
EXTRA_RELEASE_TARGETS =
for(sub, SUBDIRS) {
sub_pro = $$sub/$${basename(sub)}.pro
!exists($$sub_pro):next()
isEqual($$list($$fromfile($$sub_pro, TEMPLATE)), lib) {
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
} else { #do not have a real debug target/release
    #debug
    eval(debug-$${sub}.depends = $${sub}/$(MAKEFILE) $$EXTRA_DEBUG_TARGETS)
    eval(debug-$${sub}.commands = (cd $$sub && $(MAKE) -f $(MAKEFILE) first))
    EXTRA_DEBUG_TARGETS += debug-$${sub}
    QMAKE_EXTRA_TARGETS += debug-$${sub}
    #release
    eval(release-$${sub}.depends = $${sub}/$(MAKEFILE) $$EXTRA_RELEASE_TARGETS)
    eval(release-$${sub}.commands = (cd $$sub && $(MAKE) -f $(MAKEFILE) first))
    EXTRA_RELEASE_TARGETS += release-$${sub}
    QMAKE_EXTRA_TARGETS += release-$${sub}
}
}
debug.depends = $$EXTRA_DEBUG_TARGETS
release.depends = $$EXTRA_RELEASE_TARGETS
QMAKE_EXTRA_TARGETS += debug release
