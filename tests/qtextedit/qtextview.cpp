/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qtextview.cpp#28 $
**
** Implementation of the QTextView class
**
** Created : 990101
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qtextview.h"
#ifndef QT_NO_TEXTVIEW
#include "qrichtext_p.h"

#include "qapplication.h"
#include "qlayout.h"
#include "qpainter.h"

#include "qstack.h"
#include "stdio.h"
#include "qfile.h"
#include "qtextstream.h"
#include "qlayout.h"
#include "qbitmap.h"
#include "qtimer.h"
#include "qimage.h"
#include "qmime.h"
#include "qdragobject.h"
#include "qclipboard.h"
#include "qdragobject.h"




/*!
  \class QTextView qtextview.h
  \brief A sophisticated single-page rich text viewer.
  \ingroup basic
  \ingroup helpsystem

  Unlike QSimpleRichText, which merely draws small pieces of rich
  text, a QTextView is a real widget, with scrollbars when necessary,
  for showing large text documents.

  The rendering style and available tags are defined by a
  styleSheet(). Currently, a small XML/CSS1 subset including embedded
  images and tables is supported. See QStyleSheet for
  details. Possible images within the text document are resolved by
  using a QMimeSourceFactory.  See setMimeSourceFactory() for details.

  Using QTextView is quite similar to QLabel. It's mainly a call to
  setText() to set the contents. Setting the background color is
  slightly different from other widgets, since a text view is a
  scrollable widget that naturally provides a scrolling background. You
  can specify the colorgroup of the displayed text with
  setPaperColorGroup() or directly define the paper background with
  setPaper(). QTextView supports both plain color and complex pixmap
  backgrounds.

  Note that we do not intend to add a full-featured web browser widget
  to Qt (since that would easily double Qt's size and only few
  applications would benefit from it). In particular, the rich text
  support in Qt is supposed to provide a fast, portable and sufficient
  way to add reasonable online help facilities to applications. We
  will, however, extend it to some degree in future versions of Qt.

  For even more, like hypertext capabilities, see QTextBrowser.
*/

/*!
  Constructs an empty QTextView
  with the standard \a parent and \a name optional arguments.
*/
QTextView::QTextView(QWidget *parent, const char *name)
    : QTextEdit( parent, name )
{
    viewport()->setCursor( arrowCursor );
}


/*!
  Constructs a QTextView displaying the contents \a text with context
  \a context, with the standard \a parent and \a name optional
  arguments.
*/
QTextView::QTextView( const QString& text, const QString& context,
		      QWidget *parent, const char *name)
    : QTextEdit( parent, name )
{
    viewport()->setCursor( arrowCursor );
    setText( text, context );
}


QTextView::~QTextView()
{
}

#endif
