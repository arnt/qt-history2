/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qfocusdata.cpp#1 $
**
** Implementation of QFocusData class
**
** Created : 980622
**
** Copyright (C) 1998 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qfocusdata.h"

/*!
  \class QFocusData qfocusdata.h
  \brief Maintains the list of widgets which can take focus.

  When reimplementing QWidget::focusNextPrevChild() to provide special
  focus flow, you will usually call QWidget::focusData() to retrieve
  the focus data stored at the top-level widget - the focus data for
  that hierarchy of widgets.

  \sa QWidget::focusNextPrevChild()
*/

/*!
  \fn QWidget* QFocusData::focusWidget() const

  Returns the widgets in the hierarchy which currently has focus.
*/

/*!
  \fn int QFocusData::count() const

  Returns a count of the number of widgets in the hierarchy which accept focus.
*/

/*!
  Moves the cursor to the focusWidget() and returns that widget.
  You must call this before next() or prev() to iterate meaningfully.
*/
QWidget* QFocusData::home()
{
    focusWidgets.find(it.current());
    return focusWidgets.current();
}

/*!
  Moves the cursor to the right.  Note that the focus widgets
  are a \e loop of widgets.  If you keep calling next(), it will
  loop, without ever returning 0.
*/
QWidget* QFocusData::next()
{
    QWidget* r = focusWidgets.next();
    if ( !r )
	r = focusWidgets.first();
    return r;
}

/*!
  Moves the cursor to the left.  Note that the focus widgets
  are a \e loop of widgets.  If you keep calling prev(), it will
  loop, without ever returning 0.
*/
QWidget* QFocusData::prev()
{
    QWidget* r = focusWidgets.prev();
    if ( !r )
	r = focusWidgets.last();
    return r;
}

