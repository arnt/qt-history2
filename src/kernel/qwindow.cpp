/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwindow.cpp#10 $
**
** Implementation of QWindow class
**
** Created : 931211
**
** Copyright (C) 1993-1997 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qwindow.h"
#include "qpixmap.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qwindow.cpp#10 $");


/*!
  \class QWindow qwindow.h
  \brief The QWindow class is reserved for future extensions.

  This class is not yet finished.  It will contain intelligent handling of
  menus etc. in a future Qt release.
*/


/*!
  Constructs a window named \e name, which will be a child widget of
  \e parent.

  The widget flags \e f should normally be set to zero unless you know what you
  are doing.

  These arguments are sent to the QWidget constructor.
*/

QWindow::QWindow( QWidget *parent, const char *name, WFlags f )
    : QWidget( parent, name, f )
{
    initMetaObject();
}

/*!
  Destroys the window.
*/

QWindow::~QWindow()
{
}
