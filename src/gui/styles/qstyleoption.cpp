/****************************************************************************
**
** Implementation of the QStyleOption structure.
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

#include "qstyleoption.h"

/*!
    \class QStyleOption qstyleoption.h
    \brief QStyleOption and its sub-classes describe parameters that are used
    by QStyle functions.
    \ingroup appearance

    In previous Qt versions, many QStyle functions required a QWidget parameter
    of a certain type in order for it to work correctly. This coupled the
    QStyle closely to the widgets it was styling. In Qt 4.0, this restriction
    has been removed.  Instead, a QStyleOption or one of its sub-classes supply
    the information the function needs.

    QStyleOption and its sub-classes all have a version parameter. The current
    version is 0.  This version number must be specified explicitly when
    creating the structure.  The version number helps to ensure future
    expansion without ruining compatiblity with older styles.

    Contrary to the rest of Qt, there are few member functions and the access
    to the variables is direct. The "low-level" feel makes the structures use
    straight forward and empahsizes that these are parameters used by the style
    functions, not clases. As a downside it forces developers to be careful to
    initialize all the variables.

    QStyleOption and its sub-classes can be safely checked using the qt_cast()
    functions.
 */


/*!
  Construct a QStyleOption with version \a optionversion and type \a optiontype.
  Usually, you will only pass \a optionversion. The \a optiontype parameter is
  mainly used by the subclasses for RTTI information.
*/

QStyleOption::QStyleOption(int optionversion, int optiontype)
    : version(optionversion), type(optiontype)
{
}

/*!
  A convenience function that intializes some of the variables in the
  QStyleOption structure based on the widget \a w.

  The init() function assigns the rect and palette variables to \a w's rect()
  and palette() respectively. It also initializes the state variable checking
  if the widget is enabled and if it has focus.
*/
void QStyleOption::init(const QWidget *w)
{
    state = QStyle::Style_Default;
    if (w->isEnabled())
        state |= QStyle::Style_Enabled;
    if (w->hasFocus())
        state |= QStyle::Style_HasFocus;
    rect = w->rect();
    palette = w->palette();
}

