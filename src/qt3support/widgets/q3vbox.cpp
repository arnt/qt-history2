/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include "q3vbox.h"

/*!
    \class Q3VBox qvbox.h
    \brief The Q3VBox widget provides vertical geometry management of
    its child widgets.

    \compat

    All its child widgets will be placed vertically and sized
    according to their sizeHint()s.

    \img qvbox-m.png Q3VBox

    \sa Q3HBox
*/


/*!
    Constructs a vbox widget called \a name with parent \a parent and
    widget flags \a f.
 */
Q3VBox::Q3VBox( QWidget *parent, const char *name, Qt::WindowFlags f )
    :Q3HBox( false, parent, name, f )
{
}
