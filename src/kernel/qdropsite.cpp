/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qdropsite.cpp#10 $
**
** Implementation of Drag and Drop support
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the kernel
** module and therefore may only be used if the kernel module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qdropsite.h"

#ifndef QT_NO_DRAGANDDROP

#include "qwidget.h"


// NOT REVISED
/*!
  \class QDropSite qdropsite.h
  \brief Provides nothing and does nothing.

  This class exists only so that old code will not break.  It does
  nothing.  If your code uses it, you can safely delete it.

  It was used in Qt 1.x to do some drag and drop; that has since been
  folded into QWidget.

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

#endif // QT_NO_DRAGANDDROP
