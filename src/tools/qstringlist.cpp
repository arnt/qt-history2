/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstringlist.cpp#5 $
**
** Implementation of QStringList
**
** Created : 990406
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qstringlist.h"
#include "qdatastream.h"
#include "qtl.h"

/*
QDataStream &operator>>( QDataStream& s, QStringList& l )
{
  l.clear();
  
  Q_UINT32 c;
  s >> c;
  for( uint i = 0; i < c; ++i )
  {
    QString tmp;
    s >> tmp;
    l.append( tmp );
  }

  return s;
}

QDataStream &operator<<( QDataStream& s, const QStringList& l )
{
  s << (Q_UINT32)l.count();

  QStringList::ConstIterator it = l.begin();
  for( ; it != l.end(); ++it )
  {
    s << *it;
  }

  return s;
}
*/

/*!
  \class QStringList qstringlist.h
  \brief A list of strings

  This class is a list of QString objects. Like QValueList it is
  value based unlike QList. In contrast to QStrList it deals with
  real QString objects instead of character pointers. That makes
  QStringList the class of joice if you have to deal with unicode
  strings.
*/

/*! \fn QStringList::QStringList()
  Creates an empty list
*/

/*! \fn QStringList::QStringList( const QStringList& l )
  Creates a copy of the list. This function is very fast since
  QStringList is implicit shared. However, for the programmer this
  is the same as a deep copy. If this list or the original one or some
  other list referencing the same shared data is modified, then the
  modifying list makes a copy first.
*/

/*!
  Sorts the list of strings in ascending order.
  The sorting algorithm used is HeapSort which operates
  in O(n*logn).
*/
void QStringList::sort()
{
    qHeapSort(*this);
}


