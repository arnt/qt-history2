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

#ifndef QT_FEATURELEVEL
#ifdef QWS
#define QT_FEATURELEVEL 1
#else
#define QT_FEATURELEVEL 2
#endif
#endif

#if QT_FEATURELEVEL == 0
#define QT_L0 1
#define QT_L1 0
#define QT_L2 0
#elif QT_FEATURELEVEL == 1
#define QT_L0 1
#define QT_L1 1
#define QT_L2 0
#elif QT_FEATURELEVEL == 2
#define QT_L0 1
#define QT_L1 1
#define QT_L2 1
#endif

// images
#define QT_FEATURE_IMAGIO_BMP		QT_L2
#define QT_FEATURE_IMAGIO_PPM		QT_L2
#define QT_FEATURE_IMAGIO_XBM		QT_L2
#define QT_FEATURE_IMAGIO_XPM		QT_L0
#define QT_FEATURE_IMAGIO_PNG		QT_L2

#define QT_FEATURE_ASYNC_IO		QT_L2
#define QT_FEATURE_ASYNC_IMAGE_IO	QT_L2
#define QT_FEATURE_MOVIE		QT_L2 && QT_FEATURE_ASYNC_IO && QT_FEATURE_ASYNC_IMAGE_IO

// fonts
#define QT_FEATURE_TRUETYPE		QT_L2
#define QT_FEATURE_BDF			QT_L2
#define QT_FEATURE_FONTDATABASE		QT_L2

// i18n

#define QT_FEATURE_I18N			QT_L2 && !defined(QT_NO_CODECS)
#define QT_FEATURE_UNICODETABLES	QT_L2 && !defined(QT_LITE_UNICODE)

// misc
#define QT_FEATURE_MIME			QT_L1
#define QT_FEATURE_RICHTEXT		QT_L1 && QT_FEATURE_MIME
#define QT_FEATURE_DRAGANDDROP		QT_L2 && QT_FEATURE_MIME
#define QT_FEATURE_CLIPBOARD		QT_L2 && QT_FEATURE_MIME

#define QT_FEATURE_SOUND		QT_L2

#define QT_FEATURE_PROPERTIES		QT_L2

#define QT_FEATURE_QWS_CURSOR		QT_L1

// network
#define QT_FEATURE_NETWORKPROTOCOL	QT_L2
#define QT_FEATURE_NETWORKPROTOCOL_FTP	QT_L2 && QT_FEATURE_NETWORKPROTOCOL
#define QT_FEATURE_NETWORKPROTOCOL_HTTP	QT_L2 && QT_FEATURE_NETWORKPROTOCOL

// painting
#define QT_FEATURE_COLORNAMES		QT_L2
#define QT_FEATURE_TRANSFORMATIONS	QT_L2   // uses floating point

#define QT_FEATURE_PSPRINTER		QT_L2
#define QT_FEATURE_PRINTER		QT_L2 && ( defined(_WS_WIN_) || QT_FEATURE_PSPRINTER )

#define QT_FEATURE_PICTURE		QT_L2

// widgets
#define QT_FEATURE_WIDGETS		QT_L1   // 0 disables all except QWidget

#define QT_FEATURE_TEXTVIEW		QT_L1 && QT_FEATURE_WIDGETS && QT_FEATURE_RICHTEXT
#define QT_FEATURE_TEXTBROWSER		QT_L1 && QT_FEATURE_TEXTVIEW
#define QT_FEATURE_ICONVIEW		QT_L2 && QT_FEATURE_WIDGETS && QT_FEATURE_DRAGANDDROP
#define QT_FEATURE_LISTVIEW		QT_L1 && QT_FEATURE_WIDGETS
#define QT_FEATURE_CANVAS		QT_L2 && QT_FEATURE_WIDGETS
#define QT_FEATURE_DIAL			QT_L1 && QT_FEATURE_WIDGETS
#define QT_FEATURE_WORKSPACE		QT_L2 && QT_FEATURE_WIDGETS
#define QT_FEATURE_LCDNUMBER		QT_L2 && QT_FEATURE_WIDGETS

// styles
#define QT_FEATURE_STYLE_WINDOWS	QT_L1 && QT_FEATURE_WIDGETS
#define QT_FEATURE_STYLE_MOTIF		QT_L2 && QT_FEATURE_WIDGETS
#define QT_FEATURE_STYLE_CDE		QT_L2 && QT_FEATURE_STYLE_MOTIF
#define QT_FEATURE_STYLE_PLATINUM	QT_L2 && QT_FEATURE_WIDGETS
#define QT_FEATURE_STYLE_SGI		QT_L2 && QT_FEATURE_STYLE_MOTIF

// dialogs
#define QT_FEATURE_DIALOGS		QT_L1 && QT_FEATURE_WIDGETS

#define QT_FEATURE_FILEDIALOG		QT_L2 && QT_FEATURE_DIALOGS && QT_FEATURE_NETWORKPROTOCOL && QT_FEATURE_LISTVIEW
#define QT_FEATURE_FONTDIALOG		QT_L2 && QT_FEATURE_DIALOGS && QT_FEATURE_FONTDATABASE
#define QT_FEATURE_COLORDIALOG		QT_L2 && QT_FEATURE_DIALOGS
#define QT_FEATURE_PRINTDIALOG		QT_L2 && QT_FEATURE_DIALOGS && QT_FEATURE_LISTVIEW
#define QT_FEATURE_INPUTDIALOG		QT_L2 && QT_FEATURE_DIALOGS
#define QT_FEATURE_MESSAGEBOX		QT_L1 && QT_FEATURE_DIALOGS
#define QT_FEATURE_PROGRESSDIALOG	QT_L2 && QT_FEATURE_DIALOGS
#define QT_FEATURE_TABDIALOG		QT_L1 && QT_FEATURE_DIALOGS
#define QT_FEATURE_WIZARD		QT_L2 && QT_FEATURE_DIALOGS

#endif // QFEATURES_H
