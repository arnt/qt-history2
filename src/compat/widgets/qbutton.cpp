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

#include "qbutton.h"
#include "qpainter.h"

/*!
    \class QButton qbutton.h
    \brief The QButton class is a compatibility base class of button
    widgets

    \ingroup abstractwidgets

    <b>In new code, use QAbstractButton.</b>

    To subclass QButton, you must reimplement at least drawButton()
    (to draw the button's outline) and drawButtonLabel() (to draw its
    text or pixmap). It is generally advisable to reimplement
    sizeHint() as well, and sometimes hitButton() (to determine
    whether a button press is within the button).
*/

/*!
    Constructs a standard button called \a name with parent \a parent,
    using the widget flags \a f.
*/

QButton::QButton( QWidget *parent, const char *name, Qt::WFlags f )
    : QAbstractButton( parent )
{
    setWFlags(f);
    setObjectName(name);
}

/*!
    Destroys the button.
 */
QButton::~QButton()
{
}

/*!
    \fn void QButton::paintEvent( QPaintEvent *event)

    Handles paint events, received in \a event, for buttons. Small and
    typically complex buttons are painted double-buffered to reduce
    flicker. The actually drawing is done in the virtual functions
    drawButton() and drawButtonLabel().

    \sa drawButton(), drawButtonLabel()
*/
void QButton::paintEvent( QPaintEvent *)
{
    QPainter p(this);
    drawButton( &p );
}

/*!
    \fn void QButton::drawButton( QPainter *painter)

    Draws the button on the given \a painter. The default
    implementation does nothing.

    This virtual function is reimplemented by subclasses to draw real
    buttons. At some point, these reimplementations should call
    drawButtonLabel().

    \sa drawButtonLabel(), paintEvent()
*/
void QButton::drawButton( QPainter * )
{
}

/*!
    \fn void QButton::drawButtonLabel( QPainter *painter )

    Draws the button text or pixmap on the given \a painter.

    This virtual function is reimplemented by subclasses to draw real
    buttons. It is invoked by drawButton().

    \sa drawButton(), paintEvent()
*/

void QButton::drawButtonLabel( QPainter * )
{
}
