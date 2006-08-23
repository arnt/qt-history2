TEMPLATE      = subdirs
unset(EXAMPLES_ITEMVIEWS_SUBDIRS)
EXAMPLES_ITEMVIEWS_SUBDIRS = examples_itemviews_basicsortfiltermodel \
                             examples_itemviews_chart \
                             examples_itemviews_customsortfiltermodel \
                             examples_itemviews_dirview \
                             examples_itemviews_pixelator \
                             examples_itemviews_puzzle \
                             examples_itemviews_simpledommodel \
                             examples_itemviews_simpletreemodel \
                             examples_itemviews_spinboxdelegate

# install
EXAMPLES_ITEMVIEWS_install_sources.files = README *.pro
EXAMPLES_ITEMVIEWS_install_sources.path = $$[QT_INSTALL_EXAMPLES]/itemviews
INSTALLS += EXAMPLES_ITEMVIEWS_install_sources

#subdirs
examples_itemviews_basicsortfiltermodel.subdir = $$QT_BUILD_TREE/examples/itemviews/basicsortfiltermodel
examples_itemviews_basicsortfiltermodel.depends =  src_corelib src_gui
examples_itemviews_chart.subdir = $$QT_BUILD_TREE/examples/itemviews/chart
examples_itemviews_chart.depends =  src_corelib src_gui
examples_itemviews_customsortfiltermodel.subdir = $$QT_BUILD_TREE/examples/itemviews/customsortfiltermodel
examples_itemviews_customsortfiltermodel.depends =  src_corelib src_gui
examples_itemviews_dirview.subdir = $$QT_BUILD_TREE/examples/itemviews/dirview
examples_itemviews_dirview.depends =  src_corelib src_gui
examples_itemviews_pixelator.subdir = $$QT_BUILD_TREE/examples/itemviews/pixelator
examples_itemviews_pixelator.depends =  src_corelib src_gui
examples_itemviews_puzzle.subdir = $$QT_BUILD_TREE/examples/itemviews/puzzle
examples_itemviews_puzzle.depends =  src_corelib src_gui
examples_itemviews_simpledommodel.subdir = $$QT_BUILD_TREE/examples/itemviews/simpledommodel
examples_itemviews_simpledommodel.depends =  src_corelib src_gui src_xml
examples_itemviews_simpletreemodel.subdir = $$QT_BUILD_TREE/examples/itemviews/simpletreemodel
examples_itemviews_simpletreemodel.depends =  src_corelib src_gui
examples_itemviews_spinboxdelegate.subdir = $$QT_BUILD_TREE/examples/itemviews/spinboxdelegate
examples_itemviews_spinboxdelegate.depends =  src_corelib src_gui
EXAMPLES_SUB_SUBDIRS += $$EXAMPLES_ITEMVIEWS_SUBDIRS
SUBDIRS += $$EXAMPLES_ITEMVIEWS_SUBDIRS
