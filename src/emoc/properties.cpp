/**
  * Format of the array:
  * "WidgetName", "Comment", "PixmapFileName", "InspectorClass",
  *   "returntype", "name", "qualifier",
  *     "arg1", "arg2", ..., ")"
  *   "enum", "name", "enum1", "enum2", ..., ";"
  * "}"
  *
  */
static const char* TorbensHack[] = {
  "+", "QWidget", "A simple Widget", "", "",

  "+", "QButton", "An abstract Button", "", "",
    "void", "setText", "", "const QString&", ")",
    "QString", "text", "const", ")",

  "+", "QPushButton", "A push button", "", "",

  "+", "QGridLayout", "A grid layout", "", "",

  0
};

static const char* TorbensLayout[] = {
  "QLayout", "QGridLayout", "QHBoxLayout", "QVBoxLayout", 0
};
