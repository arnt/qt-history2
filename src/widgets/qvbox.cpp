/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qvbox.cpp#11 $
**
** Implementation of vertical box layout widget class
**
** Created : 990124
**
** Copyright (C) 1992-2000 Troll Tech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.  This file is part of the widgets
** module and therefore may only be used if the widgets module is specified
** as Licensed on the Licensee's License Certificate.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing, or see
** http://www.trolltech.com/qpl/ for QPL licensing information.
**
*****************************************************************************/


#include "qvbox.h"
#ifndef QT_NO_COMPLEXWIDGETS
#include "qlayout.h"

// NOT REVISED
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
#endif
