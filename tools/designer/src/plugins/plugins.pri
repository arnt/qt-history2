CONFIG += designer debug_and_release
DESTDIR = $$QT_BUILD_TREE/plugins/designer
contains(TEMPLATE,lib) {
   CONFIG(debug, debug|release) {
      unix:TARGET = $$member(TARGET, 0)_debug
      else:TARGET = $$member(TARGET, 0)d
   }
}

# install
target.path = $$[QT_INSTALL_PLUGINS]/designer
INSTALLS += target
