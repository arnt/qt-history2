/****************************************************************************
** $Id: //depot/qt/main/src/tools/qglobal.h#171 $
**
** Global feature selection
**
** Created : 000417
**
** Copyright (C) 2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#ifndef QFEATURES_H
#define QFEATURES_H

/*! \page features....html
    ...
*/

#include <qconfig.h>


// Data structures
/*!
    QStringList
*/
//#define QT_NO_STRINGLIST

// File I/O
//#define QT_NO_DIR
//#define QT_NO_TEXTSTREAM
//#define QT_NO_DATASTREAM

// Images
/*!
    BMP image I/O
    <p>The Windows Bitmap (BMP) image format is common on MS-Windows.
    <p>This is an uncompressed image format
    offering few advantages over PNG or JPEG.
*/
//#define QT_NO_IMAGEIO_BMP
/*!
    PPM image I/O
    <p>The Portable PixMap (PPM) image format is common on Unix.
    <p>This is an uncompressed image format
    offering few advantages over PNG or JPEG.
*/
//#define QT_NO_IMAGEIO_PPM
/*!
    XBM image I/O
    <p>The X11 BitMap (XBM) image format is common on X11.
    <p>This is an uncompressed monochrome image format.
    Qt uses this format for some internal images (eg. mouse cursors).
*/
//#define QT_NO_IMAGEIO_XBM
/*!
    XPM image I/O
    <p>The X11 PixMap (XPM) image format is common on X11.
    <p>This is an uncompressed image format.
    XPM images have the small advantage that they can be trivially
    included in source files as they are C code.
    Qt uses this format for some internal images (eg. QMessageBox icons).
*/
//#define QT_NO_IMAGEIO_XPM
/*!
    PNG image I/O
    <p>The Portable Network Graphics (PNG) is a compressed image format.
    <p>See <a href=http://www.libpng.org/pub/png/>The PNG Home Site</a> for
    details of the format.
*/
//#define QT_NO_IMAGEIO_PNG
/*!
    MNG image I/O
    <p>The Multiple-image Network Graphics (MNG) is a compressed animation format.
    <p>See <a href=http://www.libpng.org/pub/mng/>The MNG Home Site</a> for
    details of the format.
*/
//#define QT_NO_IMAGEIO_MNG
/*!
    JPEG image I/O
    <p>The Joint Photographic Experts Group (JPEG) is a compressed lossy image format that gives high compression
	for real-world and photo-realistic images.
    <p><b>Note: this currently also requires <tt>QT_JPEG_SUPPORT</tt> to
	be defined when building Qt.</b>
*/
//#define QT_NO_IMAGEIO_JPEG // currently also requires QT_JPEG_SUPPORT

/*!
    Asynchronous I/O
    <p>Allows push-driven data processing.
*/
//#define QT_NO_ASYNC_IO
/*!
    Asynchronous image I/O
    <p>Allows push-driven images.
*/
//#define QT_NO_ASYNC_IMAGE_IO
#if defined(QT_NO_ASYNC_IO) || defined(QT_NO_ASYNC_IMAGE_IO)
    /*!
	Animated images
	<p>This includes animated GIFs.
	<p><b>Note: this currently also requires <tt>QT_BUILTIN_GIF_READER</tt> to
	be defined when building Qt.</b>
    */
    #define QT_NO_MOVIE
#endif

// Fonts
/*!
    TrueType font files
    <p>Scalable font format common on MS-Windows and becoming common on Unix.
    <p>Only supported on Qt/Embedded.
*/
//#define QT_NO_TRUETYPE
/*!
    BDF font files
    <p>The Bitmap Distribution Format (BDF) font file format, common
	on Unix.
    <p>Only supported on Qt/Embedded.
*/
//#define QT_NO_BDF
/*!
    QFontDatabase
*/
//#define QT_NO_FONTDATABASE

// Internationalization

/*!
    QObject::tr()
*/
//#define QT_NO_TRANSLATION

/*!
    QTextCodec class and subclasses
*/
//#define QT_NO_TEXTCODEC

#if defined(QT_NO_TEXTCODEC)
    /*!
	QTextCodec classes
	<p>This includes some large conversion tables.
    */
    #define QT_NO_CODECS
#endif
#if defined(QT_LITE_UNICODE)
    /*!
	Unicode property tables
	<p>These include some large tables.
    */
    #define QT_NO_UNICODETABLES
#endif

// MIME-typed data
/*!
    MIME
*/
//#define QT_NO_MIME
#if defined(QT_NO_MIME)
    /*!
	RichText (HTML) display
    */
    #define QT_NO_RICHTEXT
    /*!
	Drag and drop
    */
    #define QT_NO_DRAGANDDROP
    /*!
	Cut and paste
    */
    #define QT_NO_CLIPBOARD
#endif

// Sound
/*!
    QSound
*/
//#define QT_NO_SOUND

// Scripting
/*!
    Properties
*/
//#define QT_NO_PROPERTIES

// Qt/Embedded-specific
/*!
    Visible cursor
*/
//#define QT_NO_QWS_CURSOR
/*!
    Alpha-blended cursor
*/
//#define QT_NO_QWS_ALPHA_CURSOR
/*!
    Mach64 acceleration
*/
#define QT_NO_QWS_MACH64
/*!
    Voodoo3 acceleration
*/
#define QT_NO_QWS_VOODOO3
/*!
    Virtual frame buffer
*/
//#define QT_NO_QWS_VFB
/*!
    Remote frame buffer (VNC)
*/
#define QT_NO_QWS_VNC
/*!
    1-bit monochrome
*/
//#define QT_NO_QWS_DEPTH_1
/*!
    4-bit VGA
    Not yet implemented
*/
#define QT_NO_QWS_VGA_16
/*!
    8-bit grayscale
*/
#define QT_NO_QWS_DEPTH_8GRAYSCALE
/*!
    8-bit color
*/
//#define QT_NO_QWS_DEPTH_8
/*!
    15-bit color
*/
#define QT_NO_QWS_DEPTH_15
/*!
    16-bit color
*/
//#define QT_NO_QWS_DEPTH_16
/*!
    32-bit color
*/
//#define QT_NO_QWS_DEPTH_32

/*!
    Window manager
*/
//#define QT_NO_QWS_MANAGER

/*!
    Saving of fonts
*/
//#define QT_NO_QWS_SAVEFONTS

/*!
    Favour code size over graphics speed
    <p>Smaller, slower code will be used for drawing operations.
    <p>Only supported on Qt/Embedded.
*/
//#define QT_NO_QWS_GFX_SPEED

/*!
    Qt/Embedded window system properties.
*/
//#define QT_NO_QWS_PROPERTIES

/*!
    Drawing utility functions
*/
//#define QT_NO_DRAWUTIL
/*!
    TrueColor QImage
*/
//#define QT_NO_IMAGE_TRUECOLOR
/*!
    Smooth QImage scaling
*/
//#define QT_NO_IMAGE_SMOOTHSCALE
/*!
    Image file text strings
*/
//#define QT_NO_IMAGE_TEXT

#if defined QT_NO_IMAGE_TRUECOLOR
    /*!
	16-bit QImage
    */
    #define QT_NO_IMAGE_16_BIT
#endif
#if defined(QT_NO_QWS_CURSOR) && defined(_WS_QWS_)
    /*!
	Cursors
    */
    #define QT_NO_CURSOR
#endif



// Networking
/*!
    DNS
*/
//#define QT_NO_DNS
/*!
    Network file access
*/
//#define QT_NO_NETWORKPROTOCOL
#if defined(QT_NO_NETWORKPROTOCOL) || defined(QT_NO_DNS)
    /*!
	FTP file access
    */
    #define QT_NO_NETWORKPROTOCOL_FTP
    /*!
	HTTP file access
    */
    #define QT_NO_NETWORKPROTOCOL_HTTP
#endif

// Painting
/*!
    Named colors
*/
//#define QT_NO_COLORNAMES
/*!
    Scaling and rotation
*/
//#define QT_NO_TRANSFORMATIONS

// Printing
/*!
    Printing
*/
//#define QT_NO_PRINTER
#if defined(QT_NO_PRINTER)
    /*!
	PostScript printing
    */
    #define QT_NO_PSPRINTER
#endif

// Metafiles
/*!
    QPicture
*/
//#define QT_NO_PICTURE

// Layout
/*!
    Automatic widget layout
*/
//#define QT_NO_LAYOUT

// Widgets
/*!
    QStyle
*/
//#define QT_NO_STYLE

/*!
    QIconSet
*/
//#define QT_NO_ICONSET

/*!
  QDialog
*/
//#define QT_NO_DIALOG
/*!
  QSemiModal
*/
//#define QT_NO_SEMIMODAL
/*!
  QFrame
*/
//#define QT_NO_FRAME

#if defined(QT_NO_FRAME) ||defined(QT_NO_PALETTE) || defined(QT_NO_STYLE) ||defined(QT_NO_DRAWUTIL)
    /*! 
      Basic widgets: QLAbel, QPushbutton, ...
    */  
    #define QT_NO_SIMPLEWIDGETS
#endif

#if defined(QT_NO_SIMPLEWIDGETS)
    /*!
      QLabel
    */
    #define QT_NO_LABEL
    /*!
      QPushButton (not implemented).
    */
    #define QT_NO_PUSHBUTTON
    /*!
      QLineEdit (not implemented).
    */
    #define QT_NO_LINEEDIT
#endif
#if  defined(QT_NO_ICONSET) || defined(QT_NO_IMAGE_SMOOTHSCALE) || defined(QT_NO_SIMPLEWIDGETS)
    /*!
	Pre-defined complex widgets
    */
    #define QT_NO_COMPLEXWIDGETS
#endif
#if defined(QT_NO_COMPLEXWIDGETS) || defined(QT_NO_RICHTEXT)
    /*!
	QTextView
    */
    #define QT_NO_TEXTVIEW
#endif
#if defined(QT_NO_TEXTVIEW)
    /*!
	QTextBrowser
    */
    #define QT_NO_TEXTBROWSER
#endif

#if defined(QT_NO_COMPLEXWIDGETS) || defined(QT_NO_DRAGANDDROP) || defined(QT_NO_HEADER)
    /*!
	QIconView
    */
    #define QT_NO_ICONVIEW
#endif
#if defined(QT_NO_HEADER)
    /*!
	QListView
    */
    #define QT_NO_LISTVIEW
#endif
#if defined(QT_NO_LISTBOX)
    /*!
	QComboBox
    */
    #define QT_NO_COMBOBOX
#endif    
#if defined(QT_NO_STYLE)
    /*!
	Windows style
    */
    #define QT_NO_STYLE_WINDOWS
    /*!
	Motif style
    */
    #define QT_NO_STYLE_MOTIF
#endif

#if defined(QT_NO_COMPLEXWIDGETS)
    /*!
	QAccel
    */
    #define QT_NO_ACCEL

    /*!
	QSizeGrip
    */
    #define QT_NO_SIZEGRIP
    /*!
	QHeader
    */
    #define QT_NO_HEADER
    /*!
	QIconView
    */
    #define QT_NO_ICONVIEW
    /*!
	QMenuBar
    */
    #define QT_NO_MENUBAR
    /*!
	QListBox
    */
    #define QT_NO_LISTBOX
    /*!
	QComboBox
    */
    #define QT_NO_COMBOBOX
    /*!
	QCanvas
    */
    #define QT_NO_CANVAS
    /*!
	QDial
    */
    #define QT_NO_DIAL
    /*!
	QWorkSpace
    */
    #define QT_NO_WORKSPACE
    /*!
	QLCDNumber
    */
    #define QT_NO_LCDNUMBER
    /*!
	QAction
    */
    #define QT_NO_ACTION
#endif
#if defined(QT_NO_COMPLEXWIDGETS) || defined(QT_NO_DIALOG)
   /*!
	Built-in dialogs
    */
    #define QT_NO_DIALOGS
#endif

#if defined(QT_NO_STYLE_WINDOWS)
    /*!
	Compact Windows style
    */
    #define QT_NO_STYLE_COMPACT
#endif

#if defined(QT_NO_STYLE_MOTIF)
    /*!
	CDE style
    */
    #define QT_NO_STYLE_CDE
    /*!
	SGI style
    */
    #define QT_NO_STYLE_SGI
#endif
#if defined(QT_NO_STYLE_WINDOWS)
    /*!
	Platinum style
    */
    #define QT_NO_STYLE_PLATINUM
#endif

#if defined(QT_NO_DIALOGS) || defined(QT_NO_LISTVIEW) || defined(QT_NO_NETWORKPROTOCOL) || defined(QT_NO_COMBOBOX)
    /*!
	QFileDialog
    */
    #define QT_NO_FILEDIALOG
#endif

#if defined(QT_NO_DIALOGS) || defined(QT_NO_FONTDATABASE) || defined(QT_NO_COMBOBOX)
    /*!
	QFontDialog
    */
    #define QT_NO_FONTDIALOG
#endif

#if defined(QT_NO_DIALOGS) || defined(QT_NO_LISTVIEW) || defined(QT_NO_PRINTER) || defined(QT_NO_COMBOBOX)
    /*!
	QPrintDialog
    */
    #define QT_NO_PRINTDIALOG
#endif

#if defined(QT_NO_DIALOGS) || defined(QT_NO_SEMIMODAL)
    /*!
	QProgressDialog
    */
    #define QT_NO_PROGRESSDIALOG
#endif
#if defined(QT_NO_DIALOGS) || defined(QT_NO_COMBOBOX)
    /*!
	QInputDialog
    */
    #define QT_NO_INPUTDIALOG
#endif
#if defined(QT_NO_DIALOGS)
    /*!
	QColorDialog
    */
    #define QT_NO_COLORDIALOG
    /*!
	QMessageBox
    */
    #define QT_NO_MESSAGEBOX
    /*!
	QTabDialog
    */
    #define QT_NO_TABDIALOG
    /*!
	QWizard
    */
    #define QT_NO_WIZARD
#endif

#if defined(QT_NO_STRINGLIST)
    // Desktop features
    /*! Session management support */
    #define QT_NO_SESSIONMANAGER
#endif


#endif // QFEATURES_H
