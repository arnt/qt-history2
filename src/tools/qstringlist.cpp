/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstringlist.cpp#11 $
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
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qstringlist.h"
#include "qdatastream.h"
#include "qtl.h"

/*!
  \class QStringList qstringlist.h
  \brief A list of strings

  \ingroup tools
  \ingroup shared

  This class is a list of QString objects. Like QValueList it is
  value based unlike QList. In contrast to QStrList it deals with
  real QString objects instead of character pointers. That makes
  QStringList the class of choice if you have to deal with unicode
  strings.

  Like QString itself, a QStringList provides implicit
  share. Therefore string lists can be passed around
  as value-parameters both fast and safe.

  Example:
  \code
  	QStringList list;

	// three different ways of appending values:
	list.append( "Torben");
	list += "Warwick";
	list << "Matthias" << "Arnt" << "Paul";

	// sort the list, Arnt's now first
	list.sort();
	
	// print it out
	for ( QStringList::Iterator it = list.begin(); it != list.end(); ++it ) {
	    printf( "%s \n", it->latin1() );
	}
  \endcode

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

Q_EXPORT QDataStream &operator>>( QDataStream & s, QStringList& l )
{
    return s >> (QValueList<QString>&)l;
}

Q_EXPORT QDataStream &operator<<( QDataStream & s, const QStringList& l )
{
    return s << (const QValueList<QString>&)l;
}

