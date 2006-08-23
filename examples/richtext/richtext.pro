TEMPLATE    = subdirs
unset(EXAMPLES_RICHTEXT_SUBDIRS)
EXAMPLES_RICHTEXT_SUBDIRS = examples_richtext_calendar \
                            examples_richtext_orderform \
                            examples_richtext_syntaxhighlighter

# install
target.path = $$[QT_INSTALL_EXAMPLES]/richtext
EXAMPLES_RICHTEXT_install_sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS richtext.pro README
EXAMPLES_RICHTEXT_install_sources.path = $$[QT_INSTALL_EXAMPLES]/richtext
INSTALLS += target EXAMPLES_RICHTEXT_install_sources

#subdirs
examples_richtext_calendar.subdir = $$QT_BUILD_TREE/examples/richtext/calendar
examples_richtext_calendar.depends =  src_corelib src_gui
examples_richtext_orderform.subdir = $$QT_BUILD_TREE/examples/richtext/orderform
examples_richtext_orderform.depends =  src_corelib src_gui
examples_richtext_syntaxhighlighter.subdir = $$QT_BUILD_TREE/examples/richtext/syntaxhighlighter
examples_richtext_syntaxhighlighter.depends =  src_corelib src_gui
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_RICHTEXT_SUBDIRS
SUBDIRS += $$EXAMPLES_RICHTEXT_SUBDIRS
