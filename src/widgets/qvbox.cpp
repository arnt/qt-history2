/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qvbox.cpp#4 $
**
** Implementation of vbox layout widget
**
** Created : 980220
**
** Copyright (C) 1996-1998 by Troll Tech AS.  All rights reserved.
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
  
  \sa QVBox and QHBox */


/*!
  Constructs a vbox widget with parent \a parent and name \a name
 */
QVBox::QVBox( QWidget *parent, const char *name, WFlags f )
    :QHBox( FALSE, parent, name, f )
{
}
