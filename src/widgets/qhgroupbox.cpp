/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qhgroupbox.cpp#3 $
**
** Implementation of QHGroupBox class
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

#include "qhgroupbox.h"

// NOT REVISED
/*!
  \class QHGroupBox qhgroupbox.h

  \brief The QHGroupBox widget organizes widgets in a group
  with one horizontal row.

  \ingroup geomanagement

  QHGroupBox is a convenience class that offers a thin layer on top of
  QGroupBox. Think of it as a QHBox that offers a frame with a title.
*/

/*!
  Constructs a horizontal group box with no title.

  The \e parent and \e name arguments are passed to the QWidget constructor.
*/
QHGroupBox::QHGroupBox( QWidget *parent, const char *name )
    : QGroupBox( 1, Vertical /* sic! */, parent, name )
{
}

/*!
  Constructs a horizontal group box with a title.

  The \e parent and \e name arguments are passed to the QWidget constructor.
*/

QHGroupBox::QHGroupBox( const QString &title, QWidget *parent,
			    const char *name )
    : QGroupBox( 1, Vertical /* sic! */, title, parent, name )
{
}

/*!
  Destructs the horizontal group box, deleting its child widgets.
*/
QHGroupBox::~QHGroupBox()
{
}
