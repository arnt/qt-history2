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


#include "qgridwidget.h"
#ifndef QT_NO_GRIDWIDGET
#include "qgridlayout.h"
#include "qapplication.h"
#include "qevent.h"
#include "qmenubar.h"
#include "qframe_p.h"

/*!
    \class QGridWidget qgridwidget.h
    \brief The QGridWidget class provides simple geometry management of its children.

    \ingroup geomanagement
    \ingroup appearance

    The grid places its widgets either in columns or in rows depending
    on its orientation.

    The number of rows \e or columns is defined in the constructor.
    All the grid's children will be placed and sized in accordance
    with their sizeHint() and sizePolicy().

    Use setMargin() to add space around the grid itself, and
    setSpacing() to add space between the widgets.

    If you just need a layout (not a widget) use QGridLayout instead.

    \img qgrid-m.png QGridWidget

    \sa QVBoxWidget QHBoxWidget
*/


class QGridWidgetPrivate : public QFramePrivate
{
    Q_DECLARE_PUBLIC(QGridWidget)
public:
    QGridWidgetPrivate() :lay(0) {}

    QGridLayout *lay;
};


/*!
    Constructs a grid widget with parent \a parent.
    If \a orientation is \c Qt::Horizontal, \a n specifies the number of
    columns. If \a orientation is \c Qt::Vertical, \a n specifies the number of
    rows. The widget flags \a f are passed to the QFrame constructor.
*/
QGridWidget::QGridWidget(int n, Qt::Orientation orientation, QWidget *parent, Qt::WFlags f)
    : QFrame(parent, f)
{
    Q_D(QGridWidget);

    d->lay = new QGridLayout(this);
    d->lay->setDefaultPositioning(n, orientation);
}



/*!
    Constructs a grid widget with parent \a parent.
    \a n specifies the number of columns. The widget flags \a f are
    passed to the QFrame constructor.
 */
QGridWidget::QGridWidget(int n, QWidget *parent, Qt::WFlags f)
    : QFrame(parent, f)
{
    Q_D(QGridWidget);
    d->lay = new QGridLayout(this);
    d->lay->setDefaultPositioning(n, Qt::Horizontal);
}


#ifdef QT3_SUPPORT
/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QGridWidget::QGridWidget(int n, Qt::Orientation orientation, QWidget *parent, const char *name, Qt::WFlags f)
    : QFrame(parent, f)
{
    Q_D(QGridWidget);
    setObjectName(name);
    d->lay = new QGridLayout(this);
    d->lay->setObjectName(name);
    d->lay->setDefaultPositioning(n, orientation);
}

/*!
    Use one of the constructors that doesn't take the \a name
    argument and then use setObjectName() instead.
*/
QGridWidget::QGridWidget(int n, QWidget *parent, const char *name, Qt::WFlags f)
    : QFrame(parent, f)
{
    Q_D(QGridWidget);
    setObjectName(name);
    d->lay = new QGridLayout(this);
    d->lay->setObjectName(name);
    d->lay->setDefaultPositioning(n, Qt::Horizontal);
}
#endif

/*!
    \internal
*/
void QGridWidget::childEvent(QChildEvent *e)
{
    Q_D(QGridWidget);
    QWidget *child = qobject_cast<QWidget*>(e->child());
    if (!child || child->isWindow())
        return;
    if (e->added()) {
        d->lay->addWidget(child);
    } else if (e->polished()) {
        QMenuBar *mb;
        if ((mb=qobject_cast<QMenuBar*>(child))) {
            d->lay->removeWidget(mb);
            d->lay->setMenuBar(mb);
        }
    }
}

/*!
    Sets the margin around the grid to \a margin (expressed in pixels).

    \sa setSpacing()
*/

void QGridWidget::setMargin(int margin)
{
    if (layout())
        layout()->setMargin(margin);
}

/*!
    Sets the spacing between the child widgets to \a spacing (expressed
    in pixels).

    \sa setMargin()
*/

void QGridWidget::setSpacing(int spacing)
{
    if (layout())
        layout()->setSpacing(spacing);
}

#endif
