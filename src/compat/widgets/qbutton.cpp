/****************************************************************************
**
** Implementation of QButton widget class.
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

#include "qbutton.h"
#include "qpainter.h"

/*!
    Constructs a standard button called \a name with parent \a parent,
    using the widget flags \a f.

    If \a parent is a QButtonGroup, this constructor calls
    QButtonGroup::insert().
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
    Handles paint events for buttons. Small and typically complex
    buttons are painted double-buffered to reduce flicker. The
    actually drawing is done in the virtual functions drawButton() and
    drawButtonLabel().

    \sa drawButton(), drawButtonLabel()
*/
void QButton::paintEvent( QPaintEvent *)
{
    QPainter p(this);
    drawButton( &p );
}

/*!
    Draws the button. The default implementation does nothing.

    This virtual function is reimplemented by subclasses to draw real
    buttons. At some point, these reimplementations should call
    drawButtonLabel().

    \sa drawButtonLabel(), paintEvent()
*/
void QButton::drawButton( QPainter * )
{
    return;
}
