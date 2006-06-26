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

#include "q3grid.h"
#include "qlayout.h"
#include "qapplication.h"

/*!
    \class Q3Grid qgrid.h
    \brief The Q3Grid widget provides simple geometry management of its children.

    \compat

    The grid places its widgets either in columns or in rows depending
    on its orientation.

    The number of rows \e or columns is defined in the constructor.
    All the grid's children will be placed and sized in accordance
    with their sizeHint() and sizePolicy().

    Use setMargin() to add space around the grid itself, and
    setSpacing() to add space between the widgets.

    \img qgrid-m.png Q3Grid

    \sa Q3VBox Q3HBox QGridLayout
*/

/*!
    \typedef Q3Grid::Direction
    \internal
*/

/*!
    Constructs a grid widget with parent \a parent, called \a name.
    If \a orient is \c Horizontal, \a n specifies the number of
    columns. If \a orient is \c Vertical, \a n specifies the number of
    rows. The widget flags \a f are passed to the Q3Frame constructor.
*/
Q3Grid::Q3Grid(int n, Qt::Orientation orient, QWidget *parent, const char *name,
               Qt::WindowFlags f)
    : Q3Frame(parent, name, f)
{
    int nCols, nRows;
    if (orient == Qt::Horizontal) {
        nCols = n;
        nRows = -1;
    } else {
        nCols = -1;
        nRows = n;
    }
    (new QGridLayout(this, nRows, nCols, 0, 0, name))->setAutoAdd(true);
}



/*!
    Constructs a grid widget with parent \a parent, called \a name.
    \a n specifies the number of columns. The widget flags \a f are
    passed to the Q3Frame constructor.
 */
Q3Grid::Q3Grid(int n, QWidget *parent, const char *name, Qt::WindowFlags f)
    : Q3Frame(parent, name, f)
{
    (new QGridLayout(this, -1, n, 0, 0, name))->setAutoAdd(true);
}


/*!
    Sets the spacing between the child widgets to \a space.
*/

void Q3Grid::setSpacing(int space)
{
    if (layout())
        layout()->setSpacing(space);
}


/*!\reimp
 */
void Q3Grid::frameChanged()
{
    if (layout())
        layout()->setMargin(frameWidth());
}


/*!
  \reimp
*/

QSize Q3Grid::sizeHint() const
{
    QWidget *mThis = (QWidget*)this;
    QApplication::sendPostedEvents(mThis, QEvent::ChildInserted);
    return Q3Frame::sizeHint();
}
