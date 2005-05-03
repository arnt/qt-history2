CONFIG += designer debug_and_release
DESTDIR = $$QT_BUILD_TREE/plugins/designer
contains(TEMPLATE,lib) {
   CONFIG(debug, debug|release) {
      unix:TARGET = $$member(TARGET, 0)_debug
      else:TARGET = $$member(TARGET, 0)d
   }
}
