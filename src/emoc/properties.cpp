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

  "+", "QLayout", "Basic layout", "", "",
  "void", "setMargin", "", "int", ")",
  "int", "margin", "const", ")",
  "void", "setSpacing", "", "int", ")",
  "int", "spacing", "const", ")",
  0
};

static const char* TorbensLayout[] = {
  "QLayout", "QGridLayout", "QHBoxLayout", "QVBoxLayout", 0
};

static const char* TorbensAbstract[] = {
  "QLayout", 0
};
