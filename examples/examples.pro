TEMPLATE      = subdirs
unset(EXAMPLES_SUBDIRS)
include(desktop/desktop.pro)
include(dialogs/dialogs.pro)
include(draganddrop/draganddrop.pro)
include(graphicsview/graphicsview.pro)
include(itemviews/itemviews.pro)
include(layouts/layouts.pro)
include(linguist/linguist.pro)
include(mainwindows/mainwindows.pro)
include(network/network.pro)
include(painting/painting.pro)
include(richtext/richtext.pro)
include(sql/sql.pro)
include(threads/threads.pro)
include(tools/tools.pro)
include(tutorial/tutorial.pro)
include(widgets/widgets.pro)
include(xml/xml.pro)
embedded: {
    include(qtopiacore/qtopiacore.pro)
}
!contains(QT_EDITION, Console):!cross_compile:contains(QT_BUILD_PARTS, tools): {
    include(designer/designer.pro)
}
contains(QT_BUILD_PARTS, tools):!cross_compile: {
    include(assistant/assistant.pro)
    include(qtestlib/qtestlib.pro)
}
contains(QT_CONFIG, opengl):  {
    include(opengl/opengl.pro)
}
contains(QT_CONFIG, qdbus):  {
    include(qdbus/qdbus.pro)
}
win32:!contains(QT_EDITION, OpenSource|Console): {
    include(activeqt/activeqt.pro)
}

# This creates a sub-examples rule
sub_examples_target.CONFIG = recursive
sub_examples_target.recurse = $$EXAMPLES_SUBDIRS $$EXAMPLES_SUB_SUBDIRS
sub_examples_target.target = sub-examples
sub_examples_target.recurse_target =
QMAKE_EXTRA_TARGETS += sub_examples_target

# install
EXAMPLES_install_sources.files = README *.pro
EXAMPLES_install_sources.path = $$[QT_INSTALL_EXAMPLES]
INSTALLS += EXAMPLES_install_sources
