/****************************************************************************
**
** Implementation of Drag and Drop support.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdropsite.h"

#ifndef QT_NO_DRAGANDDROP

#include "qwidget.h"


// NOT REVISED
/*!
  \class QDropSite qdropsite.h
  \brief The QDropSite class provides nothing and does nothing.

  \obsolete

  If your code uses it, you can safely delete it.

  It was used in Qt 1.x to do some drag and drop; that has since been
  folded into QWidget.

  For detailed information about drag-and-drop, see the QDragObject class.

  \sa QDragObject, QTextDrag, QImageDrag
*/

/*!
  Constructs a QDropSite to handle events for the widget \a self.

  Pass \c this as the \a self parameter.
  This enables dropping by calling QWidget::setAcceptDrops(TRUE).
*/
QDropSite::QDropSite( QWidget* self )
{
    self->setAcceptDrops( TRUE );
}

/*!
  Destroys the drop site.
*/
QDropSite::~QDropSite()
{
}

#endif // QT_NO_DRAGANDDROP
