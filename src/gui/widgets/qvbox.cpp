/****************************************************************************
**
** Implementation of vertical box layout widget class.
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


#include "qvbox.h"
#ifndef QT_NO_VBOX

/*!
    \class QVBox qvbox.h
    \brief The QVBox widget provides vertical geometry management of
    its child widgets.

    \ingroup geomanagement
    \ingroup appearance
    \ingroup organizers

    All its child widgets will be placed vertically and sized
    according to their sizeHint()s.

    \img qvbox-m.png QVBox

    \sa QHBox
*/


/*!
    Constructs a vbox widget called \a name with parent \a parent and
    widget flags \a f.
 */
QVBox::QVBox( QWidget *parent, const char *name, WFlags f )
    :QHBox( FALSE, parent, name, f )
{
}
#endif
