/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qvbuttongroup.cpp#1 $
**
** Implementation of QVButtonGroup class
**
** Created : 990602
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qvbuttongroup.h"

/*!
  \class QVButtonGroup qvbuttongroup.h

  \brief The QVButtonGroup widget organizes QButton widgets in a group
  with one vertical column.

  \ingroup realwidgets

  QVButtonGroup is a convenience class that offers a thin layer on top of
  QButtonGroup. Think of it as a QVBox that offers a frame with a title
  and is specifically designed for buttons.
*/

/*!  
  Constructs a vertical button group with no title.

  The \e parent and \e name arguments are passed to the QWidget constructor.
*/
QVButtonGroup::QVButtonGroup( QWidget *parent, const char *name )
    : QButtonGroup( 1, Horizontal /* sic! */, parent, name )
{
}

/*!
  Constructs a vertical button group with a title.

  The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QVButtonGroup::QVButtonGroup( const QString &title, QWidget *parent,
			    const char *name )
    : QButtonGroup( 1, Horizontal /* sic! */, title, parent, name )
{
}

/*!
  Destroys the vertical button group and its child widgets.
*/
QVButtonGroup::~QVButtonGroup()
{
}
