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

#include "qhboxwidget.h"
#ifndef QT_NO_HBOXWIDGET
#include "qboxlayout.h"
#include "qapplication.h"
#include "qevent.h"
#include "qmenubar.h"
#include "qframe_p.h"

/*!
    \class QHBoxWidget qhboxwidget.h
    \brief The QHBoxWidget class provides horizontal geometry management
    for its child widgets.

    \ingroup organizers
    \ingroup geomanagement
    \ingroup appearance

    All the horizontal box's child widgets will be placed alongside
    each other and sized according to their sizeHint()s.

    Use setMargin() to add space around the edges, and use
    setSpacing() to add space between the widgets. Use
    setStretchFactor() if you want the widgets to be different sizes
    in proportion to one another. (See \link layout.html
    Layouts\endlink for more information on stretch factors.)

    If you just need a layout (not a widget) use QHBoxLayout instead.

    \img qhbox-m.png QHBoxWidget

    \sa QVBoxWidget QGridWidget
*/


class QHBoxWidgetPrivate : public QFramePrivate
{
    Q_DECLARE_PUBLIC(QHBoxWidget)
public:
    QHBoxWidgetPrivate() :lay(0) {}

    QBoxLayout *lay;
};

#ifdef QT3_SUPPORT
/*! \obsolete
    Constructs an hbox widget with parent \a parent, called \a name.
    The parent, name and widget flags, \a f, are passed to the QFrame
    constructor.
*/
QHBoxWidget::QHBoxWidget(QWidget *parent, const char *name, Qt::WFlags f)
    :QFrame(parent, f)
{
    Q_D(QHBoxWidget);
    QString nm(name);
    setObjectName(nm);
    d->lay = new QHBoxLayout(this);
    d->lay->setMargin(0);
    d->lay->setSpacing(frameWidth());
    d->lay->setObjectName(nm);
}
#endif

/*!
    Constructs an hbox widget with parent \a parent.
    The parent and widget flags, \a f, are passed to the QFrame
    constructor.
*/
QHBoxWidget::QHBoxWidget(QWidget *parent, Qt::WFlags f)
    :QFrame(parent, f)
{
    Q_D(QHBoxWidget);
    d->lay = new QHBoxLayout(this);
    d->lay->setMargin(0);
    d->lay->setSpacing(frameWidth());
}

/*!
    Constructs a horizontal hbox if \a orientation is \c
    Qt::Horizontal, and constructs a vertical hbox (i.e. a vbox) if \a
    orientation is \c Qt::Vertical.

    This constructor is provided for the QVBox class. You should never
    need to use it directly.

    The \a parent and widget flags, \a f, are passed to the
    QFrame constructor.
*/

QHBoxWidget::QHBoxWidget(Qt::Orientation orientation, QWidget *parent , Qt::WFlags f)
    :QFrame(parent, f)
{
    Q_D(QHBoxWidget);
    d->lay = new QBoxLayout(orientation == Qt::Horizontal
                         ? QBoxLayout::LeftToRight
                         : QBoxLayout::Down, this);
    d->lay->setMargin(0);
    d->lay->setSpacing(frameWidth());
}

/*! \reimp
 */
void QHBoxWidget::childEvent(QChildEvent *e)
{
    Q_D(QHBoxWidget);
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
    Set the width of the outside border \a margin
*/

void QHBoxWidget::setMargin(int margin)
{
    if (layout()) // ### why not use this->lay?
        layout()->setMargin(margin);
}

/*!
    Sets the spacing between the child widgets to \a space.
*/

void QHBoxWidget::setSpacing(int space)
{
    if (layout()) // ### why not use this->lay?
        layout()->setSpacing(space);
}


/*!
  \reimp
*/

QSize QHBoxWidget::sizeHint() const
{
#ifdef QT3_SUPPORT
    QWidget *mThis = (QWidget*)this;
    QApplication::sendPostedEvents(mThis, QEvent::ChildInserted);
#endif
    return QFrame::sizeHint();
}

/*!
    Sets the stretch factor of widget \a w to \a stretch. Returns true if
    \a w is found. Otherwise returns false.

    \sa QBoxLayout::setStretchFactor() \link layout.html Layouts\endlink
*/
bool QHBoxWidget::setStretchFactor(QWidget* w, int stretch)
{
    Q_D(QHBoxWidget);

#ifdef QT3_SUPPORT
    QWidget *mThis = (QWidget*)this;
    QApplication::sendPostedEvents(mThis, QEvent::ChildInserted);
#endif
    return d->lay->setStretchFactor(w, stretch);
}
#endif
