/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdropsite.cpp#7 $
**
** Implementation of Drag and Drop support
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qdropsite.h"
#include "qwidget.h"


/*!
  \class QDropSite qdropsite.h
  \brief Encapsulates the requirements for implementing Drag-and-Drop dropping.

  Multiply-inherit from a QWidget subclass and this class to make a widget
  which can receive drag-and-drop events.  This slightly unusual arrangement
  is due to binary-compatibility issues.  Qt 2.0 will allow the same technique,
  but this class will be almost empty.

  Example:
  \code
    class MyLabel : public QLabel, QDropSite {
      public:
	MyLabel( QWidget* parent ) :
	    QLabel( parent ), QDropSite( this )
	{
	}

	void dragEnterEvent( QDragEnterEvent * );
	void dragMoveEvent( QDragMoveEvent * );
	void dragLeaveEvent( QDragLeaveEvent * );
	void dropEvent( QDropEvent * );
    };
  \endcode

  For detailed information about drag-and-drop, see the QDragObject class.

  \sa QDragObject, QTextDrag, QImageDrag
*/

/*!
  Constructs a QDropSite to handle events for the widget \a self.

  Pass <tt>this</tt> as the \a parent parameter.
  This enables dropping by calling QWidget::setAcceptDrops(TRUE).
*/
QDropSite::QDropSite( QWidget* self )
{
    self->setAcceptDrops( TRUE );
}

/*!
  Destructs the drop site.
*/
QDropSite::~QDropSite()
{
}

