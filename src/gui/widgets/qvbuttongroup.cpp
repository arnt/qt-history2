/****************************************************************************
**
** Implementation of QVButtonGroup class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qvbuttongroup.h"
#ifndef QT_NO_VBUTTONGROUP

/*!
  \class QVButtonGroup qvbuttongroup.h
  \brief The QVButtonGroup widget organizes QButton widgets in a
  vertical column.

  \ingroup geomanagement
  \ingroup organizers
  \ingroup appearance

  QVButtonGroup is a convenience class that offers a thin layer on top
  of QButtonGroup. Think of it as a QVBox that offers a frame with a
  title and is specifically designed for buttons.

  \img qbuttongroup-v.png QButtonGroup

  \sa QHButtonGroup
*/

/*!
    Constructs a vertical button group with no title.

    The \a parent and \a name arguments are passed on to the QWidget
    constructor.
*/
QVButtonGroup::QVButtonGroup( QWidget *parent, const char *name )
    : QButtonGroup( 1, Horizontal /* sic! */, parent, name )
{
}

/*!
    Constructs a vertical button group with the title \a title.

    The \a parent and \a name arguments are passed on to the QWidget
    constructor.
*/

QVButtonGroup::QVButtonGroup( const QString &title, QWidget *parent,
			    const char *name )
    : QButtonGroup( 1, Horizontal /* sic! */, title, parent, name )
{
}

/*!
    Destroys the vertical button group, deleting its child widgets.
*/
QVButtonGroup::~QVButtonGroup()
{
}
#endif
