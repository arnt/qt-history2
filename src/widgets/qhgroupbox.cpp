/****************************************************************************
**
** Implementation of QHGroupBox class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qhgroupbox.h"
#ifndef QT_NO_HGROUPBOX

/*!
    \class QHGroupBox qhgroupbox.h

    \brief The QHGroupBox widget organizes widgets in a group with one
    horizontal row.

    \ingroup organizers
    \ingroup geomanagement
    \ingroup appearance

    QHGroupBox is a convenience class that offers a thin layer on top
    of QGroupBox. Think of it as a QHBox that offers a frame with a
    title.

    \img qgroupboxes.png Group Boxes

    \sa QVGroupBox
*/

/*!
    Constructs a horizontal group box with no title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/
QHGroupBox::QHGroupBox( QWidget *parent, const char *name )
    : QGroupBox( 1, Vertical /* sic! */, parent, name )
{
}

/*!
    Constructs a horizontal group box with the title \a title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/

QHGroupBox::QHGroupBox( const QString &title, QWidget *parent,
			    const char *name )
    : QGroupBox( 1, Vertical /* sic! */, title, parent, name )
{
}

/*!
    Destroys the horizontal group box, deleting its child widgets.
*/
QHGroupBox::~QHGroupBox()
{
}
#endif
