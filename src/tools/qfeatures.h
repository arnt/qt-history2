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
#define QT_FEATURE_IMAGIO_BMP		1
#define QT_FEATURE_IMAGIO_PPM		1
#define QT_FEATURE_IMAGIO_XBM		1
#define QT_FEATURE_IMAGIO_XPM		1
#define QT_FEATURE_IMAGIO_PNG		1

#define QT_FEATURE_ASYNC_IO		1
#define QT_FEATURE_ASYNC_IMAGE_IO	1
#define QT_FEATURE_MOVIE		1 && QT_FEATURE_ASYNC_IO && QT_FEATURE_ASYNC_IMAGE_IO

// fonts
#define QT_FEATURE_TRUETYPE		1
#define QT_FEATURE_BDF			1
#define QT_FEATURE_FONTDATABASE		1

// i18n

#define QT_FEATURE_I18N			1 && !defined(QT_NO_CODECS)
#define QT_FEATURE_UNICODETABLES	1 && !defined(QT_LITE_UNICODE)

// misc
#define QT_FEATURE_MIME			1
#define QT_FEATURE_RICHTEXT		1 && QT_FEATURE_MIME
#define QT_FEATURE_DRAGANDDROP		1 && QT_FEATURE_MIME
#define QT_FEATURE_CLIPBOARD		1 && QT_FEATURE_MIME

#define QT_FEATURE_SOUND		1

#define QT_FEATURE_PROPERTIES		1

#define QT_FEATURE_QWS_CURSOR		1

// network
#define QT_FEATURE_NETWORKPROTOCOL	1
#define QT_FEATURE_NETWORKPROTOCOL_FTP	1 && QT_FEATURE_NETWORKPROTOCOL
#define QT_FEATURE_NETWORKPROTOCOL_HTTP	1 && QT_FEATURE_NETWORKPROTOCOL

// painting
#define QT_FEATURE_COLORNAMES		1
#define QT_FEATURE_TRANSFORMATIONS	1   // uses floating point

#define QT_FEATURE_PSPRINTER		1
#define QT_FEATURE_PRINTER		1 && ( defined(_WS_WIN_) || QT_FEATURE_PSPRINTER )

#define QT_FEATURE_PICTURE		1

// widgets
#define QT_FEATURE_WIDGETS		1   // 0 disables all except QWidget

#define QT_FEATURE_TEXTVIEW		1 && QT_FEATURE_WIDGETS && QT_FEATURE_RICHTEXT
#define QT_FEATURE_TEXTBROWSER		1 && QT_FEATURE_TEXTVIEW
#define QT_FEATURE_ICONVIEW		1 && QT_FEATURE_WIDGETS && QT_FEATURE_DRAGANDDROP
#define QT_FEATURE_LISTVIEW		1 && QT_FEATURE_WIDGETS
#define QT_FEATURE_CANVAS		1 && QT_FEATURE_WIDGETS
#define QT_FEATURE_DIAL			1 && QT_FEATURE_WIDGETS
#define QT_FEATURE_WORKSPACE		1 && QT_FEATURE_WIDGETS
#define QT_FEATURE_LCDNUMBER		1 && QT_FEATURE_WIDGETS

// styles
#define QT_FEATURE_STYLE_WINDOWS	1 && QT_FEATURE_WIDGETS
#define QT_FEATURE_STYLE_MOTIF		1 && QT_FEATURE_WIDGETS
#define QT_FEATURE_STYLE_CDE		1 && QT_FEATURE_STYLE_MOTIF
#define QT_FEATURE_STYLE_PLATINUM	1 && QT_FEATURE_WIDGETS
#define QT_FEATURE_STYLE_SGI		1 && QT_FEATURE_STYLE_MOTIF

// dialogs
#define QT_FEATURE_DIALOGS		1 && QT_FEATURE_WIDGETS

#define QT_FEATURE_FILEDIALOG		1 && QT_FEATURE_DIALOGS && QT_FEATURE_NETWORKPROTOCOL && QT_FEATURE_LISTVIEW
#define QT_FEATURE_FONTDIALOG		1 && QT_FEATURE_DIALOGS && QT_FEATURE_FONTDATABASE
#define QT_FEATURE_COLORDIALOG		1 && QT_FEATURE_DIALOGS
#define QT_FEATURE_PRINTDIALOG		1 && QT_FEATURE_DIALOGS && QT_FEATURE_LISTVIEW
#define QT_FEATURE_INPUTDIALOG		1 && QT_FEATURE_DIALOGS
#define QT_FEATURE_MESSAGEBOX		1 && QT_FEATURE_DIALOGS
#define QT_FEATURE_PROGRESSDIALOG	1 && QT_FEATURE_DIALOGS
#define QT_FEATURE_TABDIALOG		1 && QT_FEATURE_DIALOGS
#define QT_FEATURE_WIZARD		1 && QT_FEATURE_DIALOGS

#endif // QFEATURES_H
