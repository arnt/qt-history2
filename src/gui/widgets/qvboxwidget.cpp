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


#include "qvboxwidget.h"
#include "qlayout.h"
#ifndef QT_NO_VBOXWIDGET

/*!
    \class QVBoxWidget qvboxwidget.h
    \brief The QVBoxWidget widget provides vertical geometry management of
    its child widgets.

    \ingroup geomanagement
    \ingroup appearance
    \ingroup organizers

    All its child widgets will be placed vertically and sized
    according to their sizeHint()s. If you just need a layout (not a
    widget) use QVBoxLayout instead.

    \img qvbox-m.png QVBoxWidget

    \sa QHBoxWidget QGridWidget
*/


#ifdef QT3_SUPPORT
/*! \obsolete
    Constructs a vbox widget called \a name with parent \a parent and
    widget flags \a f.
 */
QVBoxWidget::QVBoxWidget(QWidget *parent, const char *name, Qt::WFlags f)
    :QHBoxWidget(Qt::Vertical, parent, f)
{
    QString nm(name);
    setObjectName(nm);
    layout()->setObjectName(nm);
}
#endif //QT3_SUPPORT

/*!
    Constructs a vbox widget with parent \a parent and
    widget flags \a f.
 */
QVBoxWidget::QVBoxWidget(QWidget *parent, Qt::WFlags f)
    :QHBoxWidget(Qt::Vertical, parent, f)
{
}

#endif
