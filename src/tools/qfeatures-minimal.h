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

/*
    *************************************************************

    WARNING:  Modifying this file makes your Qt installation
              incompatible with other Qt installations. Such
              modification is only supported on Qt/Embedded
              platforms, where reducing the size of Qt is
              important and the application-set is often fixed.

    *************************************************************
*/

/*! \page features....html
    ...
*/

// images
//#define QT_FEATURE_IMAGEIO_BMP
//#define QT_FEATURE_IMAGEIO_PPM
//#define QT_FEATURE_IMAGEIO_XBM
//#define QT_FEATURE_IMAGEIO_XPM
//#define QT_FEATURE_IMAGEIO_PNG
//#define QT_FEATURE_IMAGEIO_JPEG // currently also requires QT_JPEG_SUPPORT

//#define QT_FEATURE_ASYNC_IO
//#define QT_FEATURE_ASYNC_IMAGE_IO
#if defined(QT_FEATURE_ASYNC_IO) && defined(QT_FEATURE_ASYNC_IMAGE_IO)
    #define QT_FEATURE_MOVIE
#endif

// fonts
//#define QT_FEATURE_TRUETYPE
//#define QT_FEATURE_BDF
//#define QT_FEATURE_FONTDATABASE

// i18n

//#define QT_FEATURE_TRANSLATION
#if !defined(QT_NO_CODECS)
    //#define QT_FEATURE_I18N
#endif

#if !defined(QT_LITE_UNICODE)
    //#define QT_FEATURE_UNICODETABLES
#endif

// misc
//#define QT_FEATURE_MIME
#if defined(QT_FEATURE_MIME)
    #define QT_FEATURE_RICHTEXT
    //#define QT_FEATURE_DRAGANDDROP
    //#define QT_FEATURE_CLIPBOARD
#endif

//#define QT_FEATURE_SOUND

//#define QT_FEATURE_PROPERTIES

#if defined(_WS_QWS_)
    //#define QT_FEATURE_QWS_CURSOR
    //#define QT_FEATURE_QWS_MACH64
    //#define QT_FEATURE_QWS_VFB
    //#define QT_FEATURE_QWS_DEPTH_8GRAYSCALE
    #define QT_FEATURE_QWS_DEPTH_8
    //#define QT_FEATURE_QWS_DEPTH_15
    //#define QT_FEATURE_QWS_DEPTH_16
    //#define QT_FEATURE_QWS_DEPTH_32
    // Monochrome always defined
#endif

// network
//#define QT_FEATURE_DNS
//#define QT_FEATURE_NETWORKPROTOCOL
#if defined(QT_FEATURE_NETWORKPROTOCOL) && defined(QT_FEATURE_DNS)
    //#define QT_FEATURE_NETWORKPROTOCOL_FTP
    //#define QT_FEATURE_NETWORKPROTOCOL_HTTP
#endif

// painting
//#define QT_FEATURE_COLORNAMES
//#define QT_FEATURE_TRANSFORMATIONS

//#define QT_FEATURE_PSPRINTER
#if defined(_WS_WIN_) || defined(QT_FEATURE_PSPRINTER)
    #define QT_FEATURE_PRINTER
#endif

//#define QT_FEATURE_PICTURE

// widgets
//#define QT_FEATURE_WIDGETS

#if defined(QT_FEATURE_WIDGETS)
    #if defined(QT_FEATURE_RICHTEXT)
	//#define QT_FEATURE_TEXTVIEW
	#if defined(QT_FEATURE_TEXTVIEW)
	    #define QT_FEATURE_TEXTBROWSER
	#endif
    #endif
    #if defined(QT_FEATURE_DRAGANDDROP)
	//#define QT_FEATURE_ICONVIEW
    #endif
    #define QT_FEATURE_LISTVIEW
    //#define QT_FEATURE_CANVAS
    //#define QT_FEATURE_DIAL
    //#define QT_FEATURE_WORKSPACE
    //#define QT_FEATURE_LCDNUMBER
    //#define QT_FEATURE_ACTION

    // styles
    #define QT_FEATURE_STYLE_WINDOWS
    //#define QT_FEATURE_STYLE_MOTIF
    //#define QT_FEATURE_STYLE_PLATINUM
    #if defined(QT_FEATURE_STYLE_MOTIF)
	#define QT_FEATURE_STYLE_CDE
	#define QT_FEATURE_STYLE_SGI
    #endif

    // dialogs
    #define QT_FEATURE_DIALOGS
    #if defined(QT_FEATURE_DIALOGS)
	#if defined(QT_FEATURE_LISTVIEW) && defined(QT_FEATURE_NETWORKPROTOCOL)
	    //#define QT_FEATURE_FILEDIALOG
	#endif
	#if defined(QT_FEATURE_FONTDATABASE)
	    //#define QT_FEATURE_FONTDIALOG
	#endif
	//#define QT_FEATURE_COLORDIALOG
	#if defined(QT_FEATURE_LISTVIEW) && defined(QT_FEATURE_PRINTER)
	    //#define QT_FEATURE_PRINTDIALOG
	#endif
	#define QT_FEATURE_INPUTDIALOG
	#define QT_FEATURE_MESSAGEBOX
	//#define QT_FEATURE_PROGRESSDIALOG
	#define QT_FEATURE_TABDIALOG
	//#define QT_FEATURE_WIZARD
    #endif
#endif

#endif // QFEATURES_H
