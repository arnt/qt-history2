/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include "qvbox.h"
#include "qlayout.h"
#ifndef QT_NO_VBOX

/*!
    \class QVBox qvbox.h
    \brief The QVBox widget provides vertical geometry management of
    its child widgets.

    \ingroup geomanagement
    \ingroup appearance
    \ingroup organizers

    All its child widgets will be placed vertically and sized
    according to their sizeHint()s. If you just need a layout (not a
    widget) use QVBoxLayout instead.

    \img qvbox-m.png QVBox

    \sa QHBox QGrid
*/


#ifdef QT_COMPAT
/*! \obsolete
    Constructs a vbox widget called \a name with parent \a parent and
    widget flags \a f.
 */
QVBox::QVBox(QWidget *parent, const char *name, Qt::WFlags f)
    :QHBox(Qt::Vertical, parent, f)
{
    QString nm(name);
    setObjectName(nm);
    layout()->setObjectName(nm);
}
#endif //QT_COMPAT

/*!
    Constructs a vbox widget with parent \a parent and
    widget flags \a f.
 */
QVBox::QVBox(QWidget *parent, Qt::WFlags f)
    :QHBox(Qt::Vertical, parent, f)
{
}

#endif
