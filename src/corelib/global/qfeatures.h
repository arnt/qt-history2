// All feature and their dependencies
//
// This list is generated from $QTDIR/src/corelib/global/qfeatures.txt
//
// QAction
//#define QT_NO_ACTION

// Big Codecs (eg. CJK)
//#define QT_NO_BIG_CODECS

// Named colors
//#define QT_NO_COLORNAMES

// QCop IPC
//#define QT_NO_COP

// Cursors
//#define QT_NO_CURSOR

// QDirectPainter
//#define QT_NO_DIRECTPAINTER

// Document Object Model
//#define QT_NO_DOM

// Special widget effects (fading, scrolling)
//#define QT_NO_EFFECTS

// Freetype font engine
//#define QT_NO_FREETYPE

// Group boxes
//#define QT_NO_GROUPBOX

// QImageFormatPlugin
//#define QT_NO_IMAGEFORMATPLUGIN

// BMP image I/O
//#define QT_NO_IMAGEFORMAT_BMP

// JPEG image I/O
//#define QT_NO_IMAGEFORMAT_JPEG

// PNG image I/O
//#define QT_NO_IMAGEFORMAT_PNG

// PPM image I/O
//#define QT_NO_IMAGEFORMAT_PPM

// XBM image I/O
//#define QT_NO_IMAGEFORMAT_XBM

// QImage::createHeuristicMask()
//#define QT_NO_IMAGE_HEURISTIC_MASK

// Image file text strings
//#define QT_NO_IMAGE_TEXT

// QLCDNumber
//#define QT_NO_LCDNUMBER

// Single-line edits
//#define QT_NO_LINEEDIT

// QMessageBox
//#define QT_NO_MESSAGEBOX

// Animated images
//#define QT_NO_MOVIE

// QNetworkProxy
//#define QT_NO_NETWORKPROXY

// QPicture
//#define QT_NO_PICTURE

// Progress bars
//#define QT_NO_PROGRESSBAR

// Properties
//#define QT_NO_PROPERTIES

// Convert UUID to/from string
//#define QT_NO_QUUID_STRING

// Alpha-blended cursor
//#define QT_NO_QWS_ALPHA_CURSOR

// 
//#define QT_NO_QWS_DECORATION_DEFAULT

// Input methods
//#define QT_NO_QWS_INPUTMETHODS

// Console keyboard
//#define QT_NO_QWS_KEYBOARD

// Mouse
//#define QT_NO_QWS_MOUSE

// Autodetecting mouse driver
//#define QT_NO_QWS_MOUSE_AUTO

// Non-autodetecting mouse driver
//#define QT_NO_QWS_MOUSE_MANUAL

// Multi-process architecture
//#define QT_NO_QWS_MULTIPROCESS

// Qt/Embedded window system properties.
//#define QT_NO_QWS_PROPERTIES

// Pre-rendered fonts
//#define QT_NO_QWS_QPF

// Internal resize handler
//#define QT_NO_RESIZEHANDLER

// QRubberBand
//#define QT_NO_RUBBERBAND

// Session management
//#define QT_NO_SESSIONMANAGER

// Keyboard accelerators and shortcuts
//#define QT_NO_SHORTCUT

// QSignalMapper
//#define QT_NO_SIGNALMAPPER

// QSizeGrip
//#define QT_NO_SIZEGRIP

// Sliders
//#define QT_NO_SLIDER

// Playing sounds
//#define QT_NO_SOUND

// Spinbox control widget
//#define QT_NO_SPINWIDGET

// Splash screen widget
//#define QT_NO_SPLASHSCREEN

// QStackedWidget
//#define QT_NO_STACKEDWIDGET

// Status bars
//#define QT_NO_STATUSBAR

// QStatusTip
//#define QT_NO_STATUSTIP

// Standard template library compatiblity
//#define QT_NO_STL

// Motif style
//#define QT_NO_STYLE_MOTIF

// Windows style
//#define QT_NO_STYLE_WINDOWS

// Character set conversions
//#define QT_NO_TEXTCODEC

// Month and day names in dates
//#define QT_NO_TEXTDATE

// QTextStream
//#define QT_NO_TEXTSTREAM

// Thread support
//#define QT_NO_THREAD

// Tool tips
//#define QT_NO_TOOLTIP

// Translations via QObject::tr()
//#define QT_NO_TRANSLATION

// QUdpSocket
//#define QT_NO_UDPSOCKET

// QUrlInfo
//#define QT_NO_URLINFO

// Input validators
//#define QT_NO_VALIDATOR

// Wheel-mouse events
//#define QT_NO_WHEELEVENT

// Button groups
#if !defined(QT_NO_BUTTONGROUP) && (defined(QT_NO_GROUPBOX))
#define QT_NO_BUTTONGROUP
#endif

// Cut and paste
#if !defined(QT_NO_CLIPBOARD) && (defined(QT_NO_QWS_PROPERTIES))
#define QT_NO_CLIPBOARD
#endif

// Non-Unicode text conversions
#if !defined(QT_NO_CODECS) && (defined(QT_NO_TEXTCODEC))
#define QT_NO_CODECS
#endif

// QDate/QTime/QDateTime toString() and fromString()
#if !defined(QT_NO_DATESTRING) && (defined(QT_NO_TEXTDATE))
#define QT_NO_DATESTRING
#endif

// Dials
#if !defined(QT_NO_DIAL) && (defined(QT_NO_SLIDER))
#define QT_NO_DIAL
#endif

// QHostInfo
#if !defined(QT_NO_HOSTINFO) && (defined(QT_NO_TEXTSTREAM))
#define QT_NO_HOSTINFO
#endif

// QIcon
#if !defined(QT_NO_ICON) && (defined(QT_NO_IMAGE_HEURISTIC_MASK))
#define QT_NO_ICON
#endif

// XPM image I/O
#if !defined(QT_NO_IMAGEFORMAT_XPM) && (defined(QT_NO_TEXTSTREAM))
#define QT_NO_IMAGEFORMAT_XPM
#endif

// Popup-menus
#if !defined(QT_NO_MENU) && (defined(QT_NO_ACTION))
#define QT_NO_MENU
#endif

// External process invocation.
#if !defined(QT_NO_PROCESS) && (defined(QT_NO_THREAD))
#define QT_NO_PROCESS
#endif

// QProgressDialog
#if !defined(QT_NO_PROGRESSDIALOG) && (defined(QT_NO_PROGRESSBAR))
#define QT_NO_PROGRESSDIALOG
#endif

// Visible cursor
#if !defined(QT_NO_QWS_CURSOR) && (defined(QT_NO_CURSOR))
#define QT_NO_QWS_CURSOR
#endif

// 
#if !defined(QT_NO_QWS_DECORATION_STYLED) && (defined(QT_NO_QWS_DECORATION_DEFAULT))
#define QT_NO_QWS_DECORATION_STYLED
#endif

// The "Windows" style
#if !defined(QT_NO_QWS_DECORATION_WINDOWS) && (defined(QT_NO_QWS_DECORATION_DEFAULT))
#define QT_NO_QWS_DECORATION_WINDOWS
#endif

// Window Manager
#if !defined(QT_NO_QWS_MANAGER) && (defined(QT_NO_QWS_DECORATION_DEFAULT))
#define QT_NO_QWS_MANAGER
#endif

// Scroll bars
#if !defined(QT_NO_SCROLLBAR) && (defined(QT_NO_SLIDER))
#define QT_NO_SCROLLBAR
#endif

// Persistent application settings
#if !defined(QT_NO_SETTINGS) && (defined(QT_NO_TEXTSTREAM))
#define QT_NO_SETTINGS
#endif

// SOCKS v5 network proxy
#if !defined(QT_NO_SOCKS5) && (defined(QT_NO_NETWORKPROXY))
#define QT_NO_SOCKS5
#endif

// Splitters
#if !defined(QT_NO_SPLITTER) && (defined(QT_NO_RUBBERBAND))
#define QT_NO_SPLITTER
#endif

// CDE style
#if !defined(QT_NO_STYLE_CDE) && (defined(QT_NO_STYLE_MOTIF))
#define QT_NO_STYLE_CDE
#endif

// WindowsXP style
#if !defined(QT_NO_STYLE_WINDOWSXP) && (defined(QT_NO_STYLE_WINDOWS))
#define QT_NO_STYLE_WINDOWSXP
#endif

// Secure Execution Environment (experimental)
#if !defined(QT_NO_SXV) && (defined(QT_NO_MULTIPROCESS))
#define QT_NO_SXV
#endif

// FTP file access
#if !defined(QT_NO_FTP) && (defined(QT_NO_URLINFO) || defined(QT_NO_TEXTDATE))
#define QT_NO_FTP
#endif

// HTTP file access
#if !defined(QT_NO_HTTP) && (defined(QT_NO_HOSTINFO))
#define QT_NO_HTTP
#endif

// Shared library wrapper
#if !defined(QT_NO_LIBRARY) && (defined(QT_NO_SETTINGS))
#define QT_NO_LIBRARY
#endif

// QScrollArea
#if !defined(QT_NO_SCROLLAREA) && (defined(QT_NO_SCROLLBAR))
#define QT_NO_SCROLLAREA
#endif

// Translations via QObject::trUtf8()
#if !defined(QT_NO_TRANSLATION_UTF8) && (defined(QT_NO_TRANSLATION) || defined(QT_NO_TEXTCODEC))
#define QT_NO_TRANSLATION_UTF8
#endif

// Drag and drop
#if !defined(QT_NO_DRAGANDDROP) && (defined(QT_NO_QWS_PROPERTIES) || defined(QT_NO_IMAGEFORMAT_XPM))
#define QT_NO_DRAGANDDROP
#endif

// Spin boxes
#if !defined(QT_NO_SPINBOX) && (defined(QT_NO_SPINWIDGET) || defined(QT_NO_LINEEDIT) || defined(QT_NO_VALIDATOR))
#define QT_NO_SPINBOX
#endif

// Plastique style
#if !defined(QT_NO_STYLE_PLASTIQUE) && (defined(QT_NO_STYLE_WINDOWS) || defined(QT_NO_IMAGEFORMAT_XPM))
#define QT_NO_STYLE_PLASTIQUE
#endif

// Rich text edit
#if !defined(QT_NO_TEXTEDIT) && (defined(QT_NO_SCROLLAREA))
#define QT_NO_TEXTEDIT
#endif

// Tool-buttons
#if !defined(QT_NO_TOOLBUTTON) && (defined(QT_NO_ICON) || defined(QT_NO_ACTION))
#define QT_NO_TOOLBUTTON
#endif

// QErrorMessage
#if !defined(QT_NO_ERRORMESSAGE) && (defined(QT_NO_TEXTEDIT))
#define QT_NO_ERRORMESSAGE
#endif

// The Model/View Framework
#if !defined(QT_NO_ITEMVIEWS) && (defined(QT_NO_RUBBERBAND) || defined(QT_NO_SCROLLAREA))
#define QT_NO_ITEMVIEWS
#endif

// Server to play sound
#if !defined(QT_NO_QWS_SOUNDSERVER) && (defined(QT_NO_SOUND) || defined(QT_NO_HOSTINFO) || defined(QT_NO_QWS_MULTIPROCESS))
#define QT_NO_QWS_SOUNDSERVER
#endif

// QSyntaxHighlighter
#if !defined(QT_NO_SYNTAXHIGHLIGHTER) && (defined(QT_NO_TEXTEDIT))
#define QT_NO_SYNTAXHIGHLIGHTER
#endif

// Tab-bars
#if !defined(QT_NO_TABBAR) && (defined(QT_NO_TOOLBUTTON))
#define QT_NO_TABBAR
#endif

// QTextBrowser
#if !defined(QT_NO_TEXTBROWSER) && (defined(QT_NO_TEXTEDIT))
#define QT_NO_TEXTBROWSER
#endif

// QTextCodecPlugin
#if !defined(QT_NO_TEXTCODECPLUGIN) && (defined(QT_NO_TEXTCODEC) || defined(QT_NO_LIBRARY))
#define QT_NO_TEXTCODECPLUGIN
#endif

// QDirModel
#if !defined(QT_NO_DIRMODEL) && (defined(QT_NO_ITEMVIEWS))
#define QT_NO_DIRMODEL
#endif

// QListView
#if !defined(QT_NO_LISTVIEW) && (defined(QT_NO_ITEMVIEWS))
#define QT_NO_LISTVIEW
#endif

// QProxyModel
#if !defined(QT_NO_PROXYMODEL) && (defined(QT_NO_ITEMVIEWS))
#define QT_NO_PROXYMODEL
#endif

// QStandardItemModel
#if !defined(QT_NO_STANDARDITEMMODEL) && (defined(QT_NO_ITEMVIEWS))
#define QT_NO_STANDARDITEMMODEL
#endif

// QStringListModel
#if !defined(QT_NO_STRINGLISTMODEL) && (defined(QT_NO_ITEMVIEWS))
#define QT_NO_STRINGLISTMODEL
#endif

// Q3TabDialog
#if !defined(QT_NO_TABDIALOG) && (defined(QT_NO_TABBAR))
#define QT_NO_TABDIALOG
#endif

// QTableView
#if !defined(QT_NO_TABLEVIEW) && (defined(QT_NO_ITEMVIEWS))
#define QT_NO_TABLEVIEW
#endif

// QTreeView
#if !defined(QT_NO_TREEVIEW) && (defined(QT_NO_ITEMVIEWS))
#define QT_NO_TREEVIEW
#endif

// "What's this" help
#if !defined(QT_NO_WHATSTHIS) && (defined(QT_NO_TOOLBUTTON) || defined(QT_NO_ACTION))
#define QT_NO_WHATSTHIS
#endif

// QColorDialog
#if !defined(QT_NO_COLORDIALOG) && (defined(QT_NO_LINEEDIT) || defined(QT_NO_VALIDATOR) || defined(QT_NO_SPINBOX))
#define QT_NO_COLORDIALOG
#endif

// QDateTimeEdit
#if !defined(QT_NO_DATETIMEEDIT) && (defined(QT_NO_SPINBOX) || defined(QT_NO_DATESTRING))
#define QT_NO_DATETIMEEDIT
#endif

// QListWidget
#if !defined(QT_NO_LISTWIDGET) && (defined(QT_NO_LISTVIEW))
#define QT_NO_LISTWIDGET
#endif

// Menu bars
#if !defined(QT_NO_MENUBAR) && (defined(QT_NO_MENU) || defined(QT_NO_TOOLBUTTON))
#define QT_NO_MENUBAR
#endif

// QTableWidget
#if !defined(QT_NO_TABLEWIDGET) && (defined(QT_NO_TABLEVIEW))
#define QT_NO_TABLEWIDGET
#endif

// Tab widgets
#if !defined(QT_NO_TABWIDGET) && (defined(QT_NO_TABBAR) || defined(QT_NO_STACKEDWIDGET))
#define QT_NO_TABWIDGET
#endif

// QTreeWidget
#if !defined(QT_NO_TREEWIDGET) && (defined(QT_NO_TREEVIEW))
#define QT_NO_TREEWIDGET
#endif

// Main-windows
#if !defined(QT_NO_MAINWINDOW) && (defined(QT_NO_MENU) || defined(QT_NO_RESIZEHANDLER) || defined(QT_NO_TOOLBUTTON))
#define QT_NO_MAINWINDOW
#endif

// Toolbars
#if !defined(QT_NO_TOOLBAR) && (defined(QT_NO_MAINWINDOW))
#define QT_NO_TOOLBAR
#endif

// Main-windows
#if !defined(QT_NO_DOCKWIDGET) && (defined(QT_NO_RUBBERBAND) || defined(QT_NO_MAINWINDOW))
#define QT_NO_DOCKWIDGET
#endif

// Tool box
#if !defined(QT_NO_TOOLBOX) && (defined(QT_NO_ICON) || defined(QT_NO_TOOLTIP) || defined(QT_NO_TOOLBUTTON) || defined(QT_NO_SCROLLAREA))
#define QT_NO_TOOLBOX
#endif

// QComboBox
#if !defined(QT_NO_COMBOBOX) && (defined(QT_NO_LINEEDIT) || defined(QT_NO_STANDARDITEMMODEL) || defined(QT_NO_LISTVIEW))
#define QT_NO_COMBOBOX
#endif

// QInputDialog
#if !defined(QT_NO_INPUTDIALOG) && (defined(QT_NO_COMBOBOX) || defined(QT_NO_SPINBOX) || defined(QT_NO_STACKEDWIDGET))
#define QT_NO_INPUTDIALOG
#endif

// QFontDialog
#if !defined(QT_NO_FONTDIALOG) && (defined(QT_NO_STRINGLISTMODEL) || defined(QT_NO_COMBOBOX) || defined(QT_NO_VALIDATOR) || defined(QT_NO_GROUPBOX))
#define QT_NO_FONTDIALOG
#endif

// QFileDialog
#if !defined(QT_NO_FILEDIALOG) && (defined(QT_NO_DIRMODEL) || defined(QT_NO_TREEVIEW) || defined(QT_NO_MESSAGEBOX) || defined(QT_NO_COMBOBOX) || defined(QT_NO_TOOLBUTTON) || defined(QT_NO_BUTTONGROUP))
#define QT_NO_FILEDIALOG
#endif

// QWorkSpace
#if !defined(QT_NO_WORKSPACE) && (defined(QT_NO_SCROLLBAR) || defined(QT_NO_RESIZEHANDLER) || defined(QT_NO_MENU) || defined(QT_NO_TOOLBUTTON) || defined(QT_NO_MAINWINDOW) || defined(QT_NO_TOOLBAR) || defined(QT_NO_MENUBAR))
#define QT_NO_WORKSPACE
#endif

// Printing
#if !defined(QT_NO_PRINTER) && (defined(QT_NO_TEXTSTREAM) || defined(QT_NO_FILEDIALOG))
#define QT_NO_PRINTER
#endif

// QPrintDialog
#if !defined(QT_NO_PRINTDIALOG) && (defined(QT_NO_PRINTER) || defined(QT_NO_COMBOBOX) || defined(QT_NO_BUTTONGROUP) || defined(QT_NO_SPINBOX) || defined(QT_NO_TREEVIEW))
#define QT_NO_PRINTDIALOG
#endif

