/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qvbuttongroup.cpp#2 $
**
** Implementation of QVButtonGroup class
**
** Created : 990602
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
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
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qvbuttongroup.h"
#ifndef QT_NO_WIDGETS

// NOT REVISED
/*!
  \class QVButtonGroup qvbuttongroup.h

  \brief The QVButtonGroup widget organizes QButton widgets in a group
  with one vertical column.

  \ingroup geomanagement
  \ingroup organizers

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
  Destructs the vertical button group, deleting its child widgets.
*/
QVButtonGroup::~QVButtonGroup()
{
}
#endif
