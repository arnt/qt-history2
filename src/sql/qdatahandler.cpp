/****************************************************************************
**
** Implementation of QDataHandler class
**
** Created : 2000-11-03
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
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
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
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

#include "qdatahandler.h"

#ifndef QT_NO_SQL

#include "qstring.h"
#include "qmessagebox.h"

class QDataHandler::QDataHandlerPrivate
{
public:
    QDataHandlerPrivate() : mode( QDataHandler::None ), autoEd( TRUE ) {}
    QDataHandler::Mode mode;
    bool autoEd;
};

/*!
  \class QDataHandler qdatahandler.h

  \brief The QDataHandler class is an internal class for implementing
  the data-aware widgets.

  QDataHandler is a strictly internal class that acts as a base class
  for other data-aware widgets.

*/


/*!  \internal

  Constructs an empty data handler

*/

QDataHandler::QDataHandler()
{
    d = new QDataHandlerPrivate();
}


/*! \internal

  Destroys the object and frees any allocated resources.

*/

QDataHandler::~QDataHandler()
{
    delete d;
}


/*!  \internal

  Virtual function which is called when an error has occurred The
  default implementation displays a warning message to the user with
  information about the error.

*/
void QDataHandler::handleError( const QSqlError& e )
{
    QMessageBox::warning ( 0, "Warning", e.driverText() + "\n" + e.databaseText(),
			   0, 0 );
}


/*! \internal

  Sets the internal mode to \a m.

*/

void QDataHandler::setMode( QDataHandler::Mode m )
{
    d->mode = m;
}


/*! Returns the current mode.

*/

QDataHandler::Mode QDataHandler::mode() const
{
    return d->mode;
}


/*! Sets the auto-edit mode to \a auto.

*/

void QDataHandler::setAutoEditMode( bool autoEdit )
{
    d->autoEd = autoEdit;
}



/*! Returns TRUE if auto-edit mode is enabled, otherwise FALSE is
  returned.

*/

bool QDataHandler::autoEditMode() const
{
    return d->autoEd;
}

#endif
