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
  "void", "setCaption", "", "const QString&", ")",
  "QString", "caption", "const", ")",

  "+", "QButton", "An abstract Button", "", "",
    "void", "setText", "", "const QString&", ")",
    "QString", "text", "const", ")",

  "+", "QLabel", "", "", "",
    "void", "setText", "", "const QString&", ")",
    "QString", "text", "const", ")",

  "+", "QPushButton", "A push button", "", "",
  "+", "QRadioButton", "", "", "",
  "+", "QWizard", "", "", "",
  "+", "QLineEdit", "", "", "",
  "+", "QDialog", "", "", "",
  "+", "QMainWindow", "", "", "",
  "+", "QToolBar", "", "", "",
  "+", "QToolButton", "", "", "",
  "+", "QListView", "", "", "",
  "+", "QGroupBox", "", "", "",
  "+", "QMultiLineEdit", "", "", "",

  "+", "QGridLayout", "A grid layout", "", "",
  "+", "QVBoxLayout", "", "", "",
  "+", "QHBoxLayout", "", "", "",

  "+", "QLayout", "Basic layout", "", "",
  "void", "setMargin", "", "int", ")",
  "int", "margin", "const", ")",
  "void", "setSpacing", "", "int", ")",
  "int", "spacing", "const", ")",

  "+", "DGridLayout", "A qbuilder grid layout", "", "",
  "+", "DFormWidget", "A qbuilder grid layout", "", "",
  "+", "DMenuListView", "", "", "",
  "+", "DSeparator", "", "", "",
  "+", "DListView", "", "", "",

  "+", "DStretch", "", "", "",
  "void", "setStretch", "", "int", ")",
  "int", "stretch", "const", ")",

  "+", "DSpacing", "", "", "",
  "void", "setSpacing", "", "int", ")",
  "int", "spacing", "const", ")",

  0
};

static const char* TorbensLayout[] = {
  "QLayout", "QGridLayout", "QHBoxLayout", "QVBoxLayout",
  "DGridLayout", 0
};

static const char* TorbensAbstract[] = {
  "QLayout", 0
};
