/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qvbox.cpp#10 $
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


#include "qvbox.h"
#include "qlayout.h"

/*!
  \class QVBox qvbox.h
  \brief The QVBox widget performs geometry management on its children

  \ingroup geomanagement

  All its children will be placed vertically and sized
  according to their sizeHint()s.

  <img src=qvbox-m.png>

  \sa QVBox and QHBox */


/*!
  Constructs a vbox widget with parent \a parent and name \a name
 */
QVBox::QVBox( QWidget *parent, const char *name, WFlags f, bool allowLines )
    :QHBox( FALSE, parent, name, f, allowLines )
{
}
