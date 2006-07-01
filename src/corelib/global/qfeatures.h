/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
 * All features and their dependencies.
 *
 * This list is generated from $QTDIR/src/corelib/global/qfeatures.txt
 */

// QAction
//#define QT_NO_ACTION

// Big Codecs
//#define QT_NO_BIG_CODECS

// Color Names
//#define QT_NO_COLORNAMES

// QCopChannel
//#define QT_NO_COP

// QCursor
//#define QT_NO_CURSOR

// QDesktopServices
//#define QT_NO_DESKTOPSERVICES

// QDirectPainter
//#define QT_NO_DIRECTPAINTER

// Document Object Model
//#define QT_NO_DOM

// Effects
//#define QT_NO_EFFECTS

// Freetype Font Engine
//#define QT_NO_FREETYPE

// QGroupBox
//#define QT_NO_GROUPBOX

// QIcon
//#define QT_NO_ICON

// QImageIOPlugin
//#define QT_NO_IMAGEFORMATPLUGIN

// BMP Image Format
//#define QT_NO_IMAGEFORMAT_BMP

// JPEG Image Format
//#define QT_NO_IMAGEFORMAT_JPEG

// PNG Image Format
//#define QT_NO_IMAGEFORMAT_PNG

// PPM Image Format
//#define QT_NO_IMAGEFORMAT_PPM

// XBM Image Format
//#define QT_NO_IMAGEFORMAT_XBM

// QImage::createHeuristicMask()
//#define QT_NO_IMAGE_HEURISTIC_MASK

// Image Text
//#define QT_NO_IMAGE_TEXT

// QLCDNumber
//#define QT_NO_LCDNUMBER

// QLineEdit
//#define QT_NO_LINEEDIT

// QMessageBox
//#define QT_NO_MESSAGEBOX

// QMovie
//#define QT_NO_MOVIE

// QNetworkProxy
//#define QT_NO_NETWORKPROXY

// QPicture
//#define QT_NO_PICTURE

// QProgressBar
//#define QT_NO_PROGRESSBAR

// Properties
//#define QT_NO_PROPERTIES

//  Universally Unique Identifier Convertion
//#define QT_NO_QUUID_STRING

// Alpha Cursor
//#define QT_NO_QWS_ALPHA_CURSOR

// Decoration
//#define QT_NO_QWS_DECORATION_DEFAULT

// QWSInputMethod
//#define QT_NO_QWS_INPUTMETHODS

// Keyboard
//#define QT_NO_QWS_KEYBOARD

// Mouse
//#define QT_NO_QWS_MOUSE

// Mouse (Auto)
//#define QT_NO_QWS_MOUSE_AUTO

// Mouse (Non-Auto)
//#define QT_NO_QWS_MOUSE_MANUAL

// Multi-Process
//#define QT_NO_QWS_MULTIPROCESS

// Properties
//#define QT_NO_QWS_PROPERTIES

// Qt Prerendered Font Format
//#define QT_NO_QWS_QPF

// Resize Handler
//#define QT_NO_RESIZEHANDLER

// QRubberBand
//#define QT_NO_RUBBERBAND

// Session Manager
//#define QT_NO_SESSIONMANAGER

// QShortcut
//#define QT_NO_SHORTCUT

// QSignalMapper
//#define QT_NO_SIGNALMAPPER

// QSizeGrip
//#define QT_NO_SIZEGRIP

// QSlider
//#define QT_NO_SLIDER

// Sounds
//#define QT_NO_SOUND

// Spin Widget
//#define QT_NO_SPINWIDGET

// Splash screen widget
//#define QT_NO_SPLASHSCREEN

// QStackedWidget
//#define QT_NO_STACKEDWIDGET

// QStatusBar
//#define QT_NO_STATUSBAR

// Status Tip
//#define QT_NO_STATUSTIP

// Standard Template Library
//#define QT_NO_STL

// QMotifStyle
//#define QT_NO_STYLE_MOTIF

// QWindowsStyle
//#define QT_NO_STYLE_WINDOWS

// QTabletEvent
//#define QT_NO_TABLETEVENT

// QTextCodec
//#define QT_NO_TEXTCODEC

// Text Date
//#define QT_NO_TEXTDATE

// QTextStream
//#define QT_NO_TEXTSTREAM

// QThread
//#define QT_NO_THREAD

// QToolTip
//#define QT_NO_TOOLTIP

// Translation
//#define QT_NO_TRANSLATION

// QUdpSocket
//#define QT_NO_UDPSOCKET

// QUrlInfo
//#define QT_NO_URLINFO

// QValidator
//#define QT_NO_VALIDATOR

// QWheelEvent
//#define QT_NO_WHEELEVENT

// QButtonGroup
#if !defined(QT_NO_BUTTONGROUP) && (defined(QT_NO_GROUPBOX))
#define QT_NO_BUTTONGROUP
#endif

// QClipboard
#if !defined(QT_NO_CLIPBOARD) && (defined(QT_NO_QWS_PROPERTIES))
#define QT_NO_CLIPBOARD
#endif

// Codecs
#if !defined(QT_NO_CODECS) && (defined(QT_NO_TEXTCODEC))
#define QT_NO_CODECS
#endif

// QDate/QTime/QDateTime
#if !defined(QT_NO_DATESTRING) && (defined(QT_NO_TEXTDATE))
#define QT_NO_DATESTRING
#endif

// QDial
#if !defined(QT_NO_DIAL) && (defined(QT_NO_SLIDER))
#define QT_NO_DIAL
#endif

// QHostInfo
#if !defined(QT_NO_HOSTINFO) && (defined(QT_NO_TEXTSTREAM))
#define QT_NO_HOSTINFO
#endif

// XPM Image Format
#if !defined(QT_NO_IMAGEFORMAT_XPM) && (defined(QT_NO_TEXTSTREAM))
#define QT_NO_IMAGEFORMAT_XPM
#endif

// QMenu
#if !defined(QT_NO_MENU) && (defined(QT_NO_ACTION))
#define QT_NO_MENU
#endif

// QPrinter
#if !defined(QT_NO_PRINTER) && (defined(QT_NO_TEXTSTREAM))
#define QT_NO_PRINTER
#endif

// QProcess
#if !defined(QT_NO_PROCESS) && (defined(QT_NO_THREAD))
#define QT_NO_PROCESS
#endif

// QProgressDialog
#if !defined(QT_NO_PROGRESSDIALOG) && (defined(QT_NO_PROGRESSBAR))
#define QT_NO_PROGRESSDIALOG
#endif

// Cursor
#if !defined(QT_NO_QWS_CURSOR) && (defined(QT_NO_CURSOR))
#define QT_NO_QWS_CURSOR
#endif

// Decoration (Styled)
#if !defined(QT_NO_QWS_DECORATION_STYLED) && (defined(QT_NO_QWS_DECORATION_DEFAULT))
#define QT_NO_QWS_DECORATION_STYLED
#endif

// Decoration (Windows Style)
#if !defined(QT_NO_QWS_DECORATION_WINDOWS) && (defined(QT_NO_QWS_DECORATION_DEFAULT))
#define QT_NO_QWS_DECORATION_WINDOWS
#endif

// Manager
#if !defined(QT_NO_QWS_MANAGER) && (defined(QT_NO_QWS_DECORATION_DEFAULT))
#define QT_NO_QWS_MANAGER
#endif

// QScrollBar
#if !defined(QT_NO_SCROLLBAR) && (defined(QT_NO_SLIDER))
#define QT_NO_SCROLLBAR
#endif

// QSettings
#if !defined(QT_NO_SETTINGS) && (defined(QT_NO_TEXTSTREAM))
#define QT_NO_SETTINGS
#endif

//  SOCKS5
#if !defined(QT_NO_SOCKS5) && (defined(QT_NO_NETWORKPROXY))
#define QT_NO_SOCKS5
#endif

// QSplitter
#if !defined(QT_NO_SPLITTER) && (defined(QT_NO_RUBBERBAND))
#define QT_NO_SPLITTER
#endif

// QCDEStyle
#if !defined(QT_NO_STYLE_CDE) && (defined(QT_NO_STYLE_MOTIF))
#define QT_NO_STYLE_CDE
#endif

// QCleanLooksStyle
#if !defined(QT_NO_STYLE_CLEANLOOKS) && (defined(QT_NO_STYLE_WINDOWS))
#define QT_NO_STYLE_CLEANLOOKS
#endif

// QWindowsXPStyle
#if !defined(QT_NO_STYLE_WINDOWSXP) && (defined(QT_NO_STYLE_WINDOWS))
#define QT_NO_STYLE_WINDOWSXP
#endif

// SXE
#if !defined(QT_NO_SXE) && (defined(QT_NO_QWS_MULTIPROCESS))
#define QT_NO_SXE
#endif

// QSystemTrayIcon
#if !defined(QT_NO_SYSTEMTRAYICON) && (defined(QT_NO_ICON))
#define QT_NO_SYSTEMTRAYICON
#endif

// Context menu
#if !defined(QT_NO_CONTEXTMENU) && (defined(QT_NO_MENU))
#define QT_NO_CONTEXTMENU
#endif

// Common UNIX Printing System
#if !defined(QT_NO_CUPS) && (defined(QT_NO_PRINTER))
#define QT_NO_CUPS
#endif

// File Transfer Protocol
#if !defined(QT_NO_FTP) && (defined(QT_NO_URLINFO) || defined(QT_NO_TEXTDATE))
#define QT_NO_FTP
#endif

// Hyper Text Transfer Protocol
#if !defined(QT_NO_HTTP) && (defined(QT_NO_HOSTINFO))
#define QT_NO_HTTP
#endif

// QLibrary
#if !defined(QT_NO_LIBRARY) && (defined(QT_NO_SETTINGS))
#define QT_NO_LIBRARY
#endif

// QScrollArea
#if !defined(QT_NO_SCROLLAREA) && (defined(QT_NO_SCROLLBAR))
#define QT_NO_SCROLLAREA
#endif

// QToolButton
#if !defined(QT_NO_TOOLBUTTON) && (defined(QT_NO_ICON) || defined(QT_NO_ACTION))
#define QT_NO_TOOLBUTTON
#endif

// Translation (UTF-8 representation)
#if !defined(QT_NO_TRANSLATION_UTF8) && (defined(QT_NO_TRANSLATION) || defined(QT_NO_TEXTCODEC))
#define QT_NO_TRANSLATION_UTF8
#endif

// Drag and drop
#if !defined(QT_NO_DRAGANDDROP) && (defined(QT_NO_QWS_PROPERTIES) || defined(QT_NO_IMAGEFORMAT_XPM))
#define QT_NO_DRAGANDDROP
#endif

// QGraphicsView
#if !defined(QT_NO_GRAPHICSVIEW) && (defined(QT_NO_SCROLLAREA))
#define QT_NO_GRAPHICSVIEW
#endif

// QSpinBox
#if !defined(QT_NO_SPINBOX) && (defined(QT_NO_SPINWIDGET) || defined(QT_NO_LINEEDIT) || defined(QT_NO_VALIDATOR))
#define QT_NO_SPINBOX
#endif

// QPlastiqueStyle
#if !defined(QT_NO_STYLE_PLASTIQUE) && (defined(QT_NO_STYLE_WINDOWS) || defined(QT_NO_IMAGEFORMAT_XPM))
#define QT_NO_STYLE_PLASTIQUE
#endif

// QTabBar
#if !defined(QT_NO_TABBAR) && (defined(QT_NO_TOOLBUTTON))
#define QT_NO_TABBAR
#endif

// QTextEdit
#if !defined(QT_NO_TEXTEDIT) && (defined(QT_NO_SCROLLAREA))
#define QT_NO_TEXTEDIT
#endif

// QErrorMessage
#if !defined(QT_NO_ERRORMESSAGE) && (defined(QT_NO_TEXTEDIT))
#define QT_NO_ERRORMESSAGE
#endif

// The Model/View Framework
#if !defined(QT_NO_ITEMVIEWS) && (defined(QT_NO_RUBBERBAND) || defined(QT_NO_SCROLLAREA))
#define QT_NO_ITEMVIEWS
#endif

// Sound Server
#if !defined(QT_NO_QWS_SOUNDSERVER) && (defined(QT_NO_SOUND) || defined(QT_NO_HOSTINFO) || defined(QT_NO_QWS_MULTIPROCESS))
#define QT_NO_QWS_SOUNDSERVER
#endif

// QSyntaxHighlighter
#if !defined(QT_NO_SYNTAXHIGHLIGHTER) && (defined(QT_NO_TEXTEDIT))
#define QT_NO_SYNTAXHIGHLIGHTER
#endif

// Q3TabDialog
#if !defined(QT_NO_TABDIALOG) && (defined(QT_NO_TABBAR))
#define QT_NO_TABDIALOG
#endif

// QTextBrowser
#if !defined(QT_NO_TEXTBROWSER) && (defined(QT_NO_TEXTEDIT))
#define QT_NO_TEXTBROWSER
#endif

// QTextCodecPlugin
#if !defined(QT_NO_TEXTCODECPLUGIN) && (defined(QT_NO_TEXTCODEC) || defined(QT_NO_LIBRARY))
#define QT_NO_TEXTCODECPLUGIN
#endif

// QWhatsThis
#if !defined(QT_NO_WHATSTHIS) && (defined(QT_NO_TOOLBUTTON) || defined(QT_NO_ACTION))
#define QT_NO_WHATSTHIS
#endif

// QDirModel
#if !defined(QT_NO_DIRMODEL) && (defined(QT_NO_ITEMVIEWS))
#define QT_NO_DIRMODEL
#endif

// QListView
#if !defined(QT_NO_LISTVIEW) && (defined(QT_NO_ITEMVIEWS))
#define QT_NO_LISTVIEW
#endif

// QMenuBar
#if !defined(QT_NO_MENUBAR) && (defined(QT_NO_MENU) || defined(QT_NO_TOOLBUTTON))
#define QT_NO_MENUBAR
#endif

// QAbstractProxyModel
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

// QTableView
#if !defined(QT_NO_TABLEVIEW) && (defined(QT_NO_ITEMVIEWS))
#define QT_NO_TABLEVIEW
#endif

// QTabWidget
#if !defined(QT_NO_TABWIDGET) && (defined(QT_NO_TABBAR) || defined(QT_NO_STACKEDWIDGET))
#define QT_NO_TABWIDGET
#endif

// QTreeView
#if !defined(QT_NO_TREEVIEW) && (defined(QT_NO_ITEMVIEWS))
#define QT_NO_TREEVIEW
#endif

// QColorDialog
#if !defined(QT_NO_COLORDIALOG) && (defined(QT_NO_LINEEDIT) || defined(QT_NO_VALIDATOR) || defined(QT_NO_SPINBOX))
#define QT_NO_COLORDIALOG
#endif

// QCompleter
#if !defined(QT_NO_COMPLETER) && (defined(QT_NO_PROXYMODEL))
#define QT_NO_COMPLETER
#endif

// QDateTimeEdit
#if !defined(QT_NO_DATETIMEEDIT) && (defined(QT_NO_SPINBOX) || defined(QT_NO_DATESTRING))
#define QT_NO_DATETIMEEDIT
#endif

// QListWidget
#if !defined(QT_NO_LISTWIDGET) && (defined(QT_NO_LISTVIEW))
#define QT_NO_LISTWIDGET
#endif

// QMainWindow
#if !defined(QT_NO_MAINWINDOW) && (defined(QT_NO_MENU) || defined(QT_NO_RESIZEHANDLER) || defined(QT_NO_TOOLBUTTON))
#define QT_NO_MAINWINDOW
#endif

// QSortFilterProxyModel
#if !defined(QT_NO_SORTFILTERPROXYMODEL) && (defined(QT_NO_PROXYMODEL))
#define QT_NO_SORTFILTERPROXYMODEL
#endif

// QTableWidget
#if !defined(QT_NO_TABLEWIDGET) && (defined(QT_NO_TABLEVIEW))
#define QT_NO_TABLEWIDGET
#endif

// QTreeWidget
#if !defined(QT_NO_TREEWIDGET) && (defined(QT_NO_TREEVIEW))
#define QT_NO_TREEWIDGET
#endif

// QToolBar
#if !defined(QT_NO_TOOLBAR) && (defined(QT_NO_MAINWINDOW))
#define QT_NO_TOOLBAR
#endif

// QDockwidget
#if !defined(QT_NO_DOCKWIDGET) && (defined(QT_NO_RUBBERBAND) || defined(QT_NO_MAINWINDOW))
#define QT_NO_DOCKWIDGET
#endif

// QToolBox
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

// QFontComboBox
#if !defined(QT_NO_FONTCOMBOBOX) && (defined(QT_NO_COMBOBOX) || defined(QT_NO_STRINGLISTMODEL))
#define QT_NO_FONTCOMBOBOX
#endif

// QFontDialog
#if !defined(QT_NO_FONTDIALOG) && (defined(QT_NO_STRINGLISTMODEL) || defined(QT_NO_COMBOBOX) || defined(QT_NO_VALIDATOR) || defined(QT_NO_GROUPBOX))
#define QT_NO_FONTDIALOG
#endif

// QPrintDialog
#if !defined(QT_NO_PRINTDIALOG) && (defined(QT_NO_PRINTER) || defined(QT_NO_COMBOBOX) || defined(QT_NO_BUTTONGROUP) || defined(QT_NO_SPINBOX) || defined(QT_NO_TREEVIEW))
#define QT_NO_PRINTDIALOG
#endif

// QWorkSpace
#if !defined(QT_NO_WORKSPACE) && (defined(QT_NO_SCROLLBAR) || defined(QT_NO_RESIZEHANDLER) || defined(QT_NO_MENU) || defined(QT_NO_TOOLBUTTON) || defined(QT_NO_MAINWINDOW) || defined(QT_NO_TOOLBAR) || defined(QT_NO_MENUBAR))
#define QT_NO_WORKSPACE
#endif

// QFileDialog
#if !defined(QT_NO_FILEDIALOG) && (defined(QT_NO_DIRMODEL) || defined(QT_NO_TREEVIEW) || defined(QT_NO_MESSAGEBOX) || defined(QT_NO_COMBOBOX) || defined(QT_NO_TOOLBUTTON) || defined(QT_NO_BUTTONGROUP))
#define QT_NO_FILEDIALOG
#endif

