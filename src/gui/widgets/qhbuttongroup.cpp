/****************************************************************************
**
** Implementation of QHButtonGroup class.
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

#include "qhbuttongroup.h"
#ifndef QT_NO_HBUTTONGROUP

/*!
    \class QHButtonGroup qhbuttongroup.h
    \brief The QHButtonGroup widget organizes QButton widgets in a
    group with one horizontal row.

    \ingroup organizers
    \ingroup geomanagement
    \ingroup appearance

    QHButtonGroup is a convenience class that offers a thin layer on
    top of QButtonGroup. From a layout point of view it is effectively
    a QHBox that offers a frame with a title and is specifically
    designed for buttons. From a functionality point of view it is a
    QButtonGroup.

    \img qbuttongroup-h.png QButtonGroup

    \sa QVButtonGroup
*/

/*!
    Constructs a horizontal button group with no title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/
QHButtonGroup::QHButtonGroup(QWidget *parent, const char *name)
    : QButtonGroup(1, Vertical /* sic! */, parent, name)
{
}

/*!
    Constructs a horizontal button group with the title \a title.

    The \a parent and \a name arguments are passed to the QWidget
    constructor.
*/

QHButtonGroup::QHButtonGroup(const QString &title, QWidget *parent,
                            const char *name)
    : QButtonGroup(1, Vertical /* sic! */, title, parent, name)
{
}

/*!
    Destroys the horizontal button group, deleting its child widgets.
*/
QHButtonGroup::~QHButtonGroup()
{
}
#endif
