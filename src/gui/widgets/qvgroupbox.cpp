/****************************************************************************
**
** Implementation of QVGroupBox class.
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

#include "qvgroupbox.h"
#ifndef QT_NO_VGROUPBOX

/*!
  \class QVGroupBox qvgroupbox.h
  \brief The QVGroupBox widget organizes a group of widgets in a
  vertical column.

  \ingroup geomanagement
  \ingroup appearance
  \ingroup organizers

  QVGroupBox is a convenience class that offers a thin layer on top of
  QGroupBox. Think of it as a QVBox that offers a frame with a title.

  \img qgroupboxes.png Group Boxes

  \sa QHGroupBox
*/

/*!
    Constructs a vertical group box with no title.

    The \a parent and \a name arguments are passed on to the QWidget
    constructor.
*/
QVGroupBox::QVGroupBox(QWidget *parent, const char *name)
    : QGroupBox(1, Horizontal /* sic! */, parent, name)
{
}

/*!
    Constructs a vertical group box with the title \a title.

    The \a parent and \a name arguments are passed on to the QWidget
    constructor.
*/

QVGroupBox::QVGroupBox(const QString &title, QWidget *parent,
                            const char *name)
    : QGroupBox(1, Horizontal /* sic! */, title, parent, name)
{
}

/*!
    Destroys the vertical group box, deleting its child widgets.
*/
QVGroupBox::~QVGroupBox()
{
}
#endif
