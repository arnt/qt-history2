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

// images
//#define QT_NO_IMAGEIO_BMP
//#define QT_NO_IMAGEIO_PPM
//#define QT_NO_IMAGEIO_XBM
//#define QT_NO_IMAGEIO_XPM
//#define QT_NO_IMAGEIO_PNG
//#define QT_NO_IMAGEIO_JPEG // currently also requires QT_JPEG_SUPPORT

//#define QT_NO_ASYNC_IO
//#define QT_NO_ASYNC_IMAGE_IO
#if defined(QT_NO_ASYNC_IO) || defined(QT_NO_ASYNC_IMAGE_IO)
    #define QT_NO_MOVIE
#endif

// fonts
//#define QT_NO_TRUETYPE
//#define QT_NO_BDF
//#define QT_NO_FONTDATABASE

// i18n

//#define QT_NO_TRANSLATION
#if defined(QT_NO_CODECS)
    #define QT_NO_I18N
#endif

#if defined(QT_LITE_UNICODE)
    #define QT_NO_UNICODETABLES
#endif

// misc
//#define QT_NO_MIME
#if defined(QT_NO_MIME)
    #define QT_NO_RICHTEXT
    #define QT_NO_DRAGANDDROP
    #define QT_NO_CLIPBOARD
#endif

//#define QT_NO_SOUND

//#define QT_NO_PROPERTIES

// Qt/Embedded-specific
//#define QT_NO_QWS_CURSOR
//#define QT_NO_QWS_ALPHA_CURSOR
#define QT_NO_QWS_MACH64
#define QT_NO_QWS_VOODOO3
//#define QT_NO_QWS_VFB
#define QT_NO_QWS_DEPTH_8GRAYSCALE
//#define QT_NO_QWS_DEPTH_8
#define QT_NO_QWS_DEPTH_15
//#define QT_NO_QWS_DEPTH_16
//#define QT_NO_QWS_DEPTH_32


//#define QT_NO_DRAWUTIL
//#define QT_NO_IMAGE_32_BIT
//#define QT_NO_IMAGE_SMOOTHSCALE
//#define QT_NO_IMAGE_TEXT

#if defined QT_NO_IMAGE_TRUECOLOR
    #define QT_NO_IMAGE_16_BIT
#endif
#if defined(QT_NO_QWS_CURSOR) && defined(_WS_QWS_)
    #define QT_NO_CURSOR
#endif



// network
//#define QT_NO_DNS
//#define QT_NO_NETWORKPROTOCOL
#if defined(QT_NO_NETWORKPROTOCOL) || defined(QT_NO_DNS)
    #define QT_NO_NETWORKPROTOCOL_FTP
    #define QT_NO_NETWORKPROTOCOL_HTTP
#endif

// painting
//#define QT_NO_COLORNAMES
//#define QT_NO_TRANSFORMATIONS

//#define QT_NO_PSPRINTER
#if defined(QT_NO_PSPRINTER) && !defined(_WS_WIN_)
    #define QT_NO_PRINTER
#endif

//#define QT_NO_PICTURE

//define QT_NO_LAYOUT

// widgets
#if defined(QT_NO_PALETTE)
    #define QT_NO_WIDGETS
#endif
#if defined(QT_NO_WIDGETS) || defined(QT_NO_RICHTEXT)
    #define QT_NO_TEXTVIEW
#endif
#if defined(QT_NO_TEXTVIEW)
    #define QT_NO_TEXTBROWSER
#endif

#if defined(QT_NO_WIDGETS) || defined(QT_NO_DRAGANDDROP)
    #define QT_NO_ICONVIEW
#endif

#if defined(QT_NO_WIDGETS)
    #define QT_NO_ACCEL
    #define QT_NO_STYLE
    #define QT_NO_ICONSET

    #define QT_NO_SIZEGRIP
    #define QT_NO_LISTVIEW
    #define QT_NO_CANVAS
    #define QT_NO_DIAL
    #define QT_NO_WORKSPACE
    #define QT_NO_LCDNUMBER
    #define QT_NO_ACTION
    #define QT_NO_DIALOGS

    // styles
    #define QT_NO_STYLE_WINDOWS
    #define QT_NO_STYLE_MOTIF
    #define QT_NO_STYLE_PLATINUM
#endif

#if defined(QT_NO_STYLE_MOTIF)
    #define QT_NO_STYLE_CDE
    #define QT_NO_STYLE_SGI
#endif

#if defined(QT_NO_DIALOGS) || defined(QT_NO_LISTVIEW) || defined(QT_NO_NETWORKPROTOCOL)
    #define QT_NO_FILEDIALOG
#endif

#if defined(QT_NO_DIALOGS) || defined(QT_NO_FONTDATABASE)
    #define QT_NO_FONTDIALOG
#endif

#if defined(QT_NO_DIALOGS) || defined(QT_NO_LISTVIEW) || defined(QT_NO_PRINTER)
    #define QT_NO_PRINTDIALOG
#endif

#if defined(QT_NO_DIALOGS)
    #define QT_NO_COLORDIALOG
    #define QT_NO_INPUTDIALOG
    #define QT_NO_MESSAGEBOX
    #define QT_NO_PROGRESSDIALOG
    #define QT_NO_TABDIALOG
    #define QT_NO_WIZARD
#endif

#endif // QFEATURES_H
