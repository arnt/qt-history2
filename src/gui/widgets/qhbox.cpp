/****************************************************************************
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

#include "qhbox.h"
#ifndef QT_NO_HBOX
#include "qlayout.h"
#include "qapplication.h"
#include "qevent.h"
#include "qmenubar.h"

/*!
    \class QHBox qhbox.h
    \brief The QHBox widget provides horizontal geometry management
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

    \img qhbox-m.png QHBox

    \sa QHBoxLayout QVBox QGrid
*/


/*!
    Constructs an hbox widget with parent \a parent, called \a name.
    The parent, name and widget flags, \a f, are passed to the QFrame
    constructor.
*/
QHBox::QHBox(QWidget *parent, const char *name, WFlags f)
    :QFrame(parent, name, f)
{
    lay = new QHBoxLayout(this, frameWidth(), frameWidth(), name);
}


/*!
    Constructs a horizontal hbox if \a horizontal is true, otherwise
    constructs a vertical hbox (also known as a vbox).

    This constructor is provided for the QVBox class. You should never
    need to use it directly.

    The \a parent, \a name and widget flags, \a f, are passed to the
    QFrame constructor.
*/

QHBox::QHBox(bool horizontal, QWidget *parent , const char *name, WFlags f)
    :QFrame(parent, name, f)
{
    lay = new QBoxLayout(this,
                       horizontal ? QBoxLayout::LeftToRight : QBoxLayout::Down,
                          frameWidth(), frameWidth(), name);
}

/*! \reimp
 */
void QHBox::childEvent(QChildEvent *e)
{
    QWidget *child = qt_cast<QWidget*>(e->child());
    if (!child || child->isTopLevel())
        return;
    if (e->added()) {
        lay->addWidget(child);
    } else if (e->polished()) {
        QMenuBar *mb;
        if ((mb=qt_cast<QMenuBar*>(child))) {
            lay->removeWidget(mb);
            lay->setMenuBar(mb);
        }
    }
}


/*!
    Sets the spacing between the child widgets to \a space.
*/

void QHBox::setSpacing(int space)
{
    if (layout()) // ### why not use this->lay?
        layout()->setSpacing(space);
}


/*!
  \reimp
*/

QSize QHBox::sizeHint() const
{
    QWidget *mThis = (QWidget*)this;
    QApplication::sendPostedEvents(mThis, QEvent::ChildInserted);
    return QFrame::sizeHint();
}

/*!
    Sets the stretch factor of widget \a w to \a stretch. Returns true if
    \a w is found. Otherwise returns false.

    \sa QBoxLayout::setStretchFactor() \link layout.html Layouts\endlink
*/
bool QHBox::setStretchFactor(QWidget* w, int stretch)
{
    QWidget *mThis = (QWidget*)this;
    QApplication::sendPostedEvents(mThis, QEvent::ChildInserted);
    return lay->setStretchFactor(w, stretch);
}
#endif
