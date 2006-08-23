TEMPLATE = subdirs
unset(EXAMPLES_TUTORIAL_SUBDIRS)
EXAMPLES_TUTORIAL_SUBDIRS = examples_tutorial_t1 \
                            examples_tutorial_t2 \
                            examples_tutorial_t3 \
                            examples_tutorial_t4 \
                            examples_tutorial_t5 \
                            examples_tutorial_t6 \
                            examples_tutorial_t7 \
                            examples_tutorial_t8 \
                            examples_tutorial_t9 \
                            examples_tutorial_t10 \
                            examples_tutorial_t11 \
                            examples_tutorial_t12 \
                            examples_tutorial_t13 \
                            examples_tutorial_t14

# install
target.path = $$[QT_INSTALL_EXAMPLES]/tutorial
EXAMPLES_TUTORIAL_install_sources.files = $$SOURCES $$HEADERS $$RESOURCES $$FORMS tutorial.pro README
EXAMPLES_TUTORIAL_install_sources.path = $$[QT_INSTALL_EXAMPLES]/tutorial
INSTALLS += target EXAMPLES_TUTORIAL_install_sources

#subdirs
examples_tutorial_t1.subdir = $$QT_BUILD_TREE/examples/tutorial/t1
examples_tutorial_t1.depends =  src_corelib src_gui
examples_tutorial_t2.subdir = $$QT_BUILD_TREE/examples/tutorial/t2
examples_tutorial_t2.depends =  src_corelib src_gui
examples_tutorial_t3.subdir = $$QT_BUILD_TREE/examples/tutorial/t3
examples_tutorial_t3.depends =  src_corelib src_gui
examples_tutorial_t4.subdir = $$QT_BUILD_TREE/examples/tutorial/t4
examples_tutorial_t4.depends =  src_corelib src_gui
examples_tutorial_t5.subdir = $$QT_BUILD_TREE/examples/tutorial/t5
examples_tutorial_t5.depends =  src_corelib src_gui
examples_tutorial_t6.subdir = $$QT_BUILD_TREE/examples/tutorial/t6
examples_tutorial_t6.depends =  src_corelib src_gui
examples_tutorial_t7.subdir = $$QT_BUILD_TREE/examples/tutorial/t7
examples_tutorial_t7.depends =  src_corelib src_gui
examples_tutorial_t8.subdir = $$QT_BUILD_TREE/examples/tutorial/t8
examples_tutorial_t8.depends =  src_corelib src_gui
examples_tutorial_t9.subdir = $$QT_BUILD_TREE/examples/tutorial/t9
examples_tutorial_t9.depends =  src_corelib src_gui
examples_tutorial_t10.subdir = $$QT_BUILD_TREE/examples/tutorial/t10
examples_tutorial_t10.depends =  src_corelib src_gui
examples_tutorial_t11.subdir = $$QT_BUILD_TREE/examples/tutorial/t11
examples_tutorial_t11.depends =  src_corelib src_gui
examples_tutorial_t12.subdir = $$QT_BUILD_TREE/examples/tutorial/t12
examples_tutorial_t12.depends =  src_corelib src_gui
examples_tutorial_t13.subdir = $$QT_BUILD_TREE/examples/tutorial/t13
examples_tutorial_t13.depends =  src_corelib src_gui
examples_tutorial_t14.subdir = $$QT_BUILD_TREE/examples/tutorial/t14
examples_tutorial_t14.depends =  src_corelib src_gui
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_TUTORIAL_SUBDIRS
SUBDIRS += $$EXAMPLES_TUTORIAL_SUBDIRS
