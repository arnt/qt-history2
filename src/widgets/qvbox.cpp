/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qvbox.cpp#11 $
**
** Implementation of vertical box layout widget class
**
** Created : 990124
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the widgets module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/


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
