/****************************************************************************
** $Id: //depot/qt/main/src/widgets/qvbox.cpp#7 $
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
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
