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
  "void", "setGeometry", "", "const QRect&", ")",
  "const QRect&", "geometry", "const", ")",

  "+", "QButton", "An abstract Button", "", "",
    "void", "setText", "", "const QString&", ")",
    "QString", "text", "const", ")",
    "void", "setPixmap", "", "const QPixmap&", ")",
    "const QPixmap*", "pixmap", "const", ")",

  "+", "QLabel", "", "", "",
    "void", "setText", "", "const QString&", ")",
    "QString", "text", "const", ")",
    "void", "setPixmap", "", "const QPixmap&", ")",
    "Qpixmap*", "pixmap", "const", ")",

  "+", "QPushButton", "A push button", "", "",
  "void", "setToggleButton", "", "bool", ")",
  "bool", "isToggleButton", "const", ")",
  "+", "QRadioButton", "", "", "",
  "+", "QWizard", "", "", "",
  "+", "QLineEdit", "", "", "",
  "+", "QDialog", "", "", "",
  "+", "QMainWindow", "", "", "",
  "void", "setCaption", "", "const QString&", ")",
  "QString", "caption", "const", ")",
  "+", "QToolBar", "", "", "",
  "void", "setLabel", "", "const QString&", ")",
  "QString", "label", "const", ")",
  "+", "QToolButton", "", "", "",
  "+", "QListView", "", "", "",
  "+", "QGroupBox", "", "", "",
  "+", "QMultiLineEdit", "", "", "",
  "+", "QPopupMenu", "", "", "",

  "+", "QFrame", "", "", "",
  "enum", "Shape",
          "NoFrame", "Box", "Panel",
	  "WinPanel", "HLine", "VLine",
	  "StyledPanel", "PopupPanel", "MShape", "}",
  "enum", "Shadow",
          "Plain", "Raised", "Sunken", "MShadow", "}",
  "void", "setFrameShape", "", "Shape", ")",
  "Shape", "frameShape", "const", ")",
  "void", "setFrameShadow", "", "Shadow", ")",
  "Shadow", "frameShadow", "const", ")",

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
