/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwindow.cpp#3 $
**
** Implementation of QWindow class
**
** Author  : Haavard Nord
** Created : 931211
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qwindow.h"
#include "qpixmap.h"

RCSTAG("$Id: //depot/qt/main/src/kernel/qwindow.cpp#3 $")


/*!
  \class QWindow qwindow.h
  \brief The QWindow class is a widget with a caption and an icon.

  A window is a widget that can have a caption (window title), an
  icon and an icon text.
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
    initMetaObject();				// initialize meta object
    ctext = qAppName();				// default caption
    ipm = 0;
}

/*!
  Destroys the window.
*/

QWindow::~QWindow()
{
    delete ipm;
}


/*!
  Returns the window caption (title).
  \sa setCaption.
*/

char *QWindow::caption() const			// get caption text
{
    return (char *)((const char*)ctext);
}

/*!
  Returns the icon text.
  \sa setIcon(), setIconText()
*/

char *QWindow::iconText() const			// get icon text
{
    return (char *)((const char*)itext);
}
