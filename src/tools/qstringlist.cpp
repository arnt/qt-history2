/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstringlist.cpp#25 $
**
** Implementation of QStringList
**
** Created : 990406
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the tools module of the Qt GUI Toolkit.
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

#include "qstringlist.h"

#ifndef QT_NO_STRINGLIST
#include "qregexp.h"
#include "qstrlist.h"
#include "qdatastream.h"
#include "qtl.h"

// NOT REVISED
/*!
  \class QStringList qstringlist.h
  \brief The QStringList class provides a list of strings.

  \ingroup tools
  \ingroup shared

  It is used to store and manipulate strings that logically belong together.
  Basically QStringList is a QValueList of QString objects. As opposed
  to QStrList, which stores pointers to characters, QStringList deals
  with real QString objects.  It is the class of choice whenever you
  work with Unicode strings.

  Like QString itself, QStringList objects are implicitly shared.
  Passing them around as value-parameters is both fast and safe.

  There are several approaches to add strings to a string list:

  \walkthrough fonts/simple-qfont-demo/viewer.cpp	

  \skipto QStringList substitutes
  \printline substitutes
  \printline append 
  \printline += 
  \printline << 

  To successively access the members of a QStringList use the provided
  Iterator:

  \walkthrough fonts/simple-qfont-demo/viewer.cpp	

  \skipto QStringList substitutions 
  \printline substitutes
  \skipto QStringList::Iterator
  \printline substitutions.begin
  \printline substitutions.end
  \printuntil }

  (Code examples taken from \link simple-font-demo-example.html 
  examples/fonts/simple-qfont-demo/viewer.cpp \endlink)

  Convenience methods such as sort(), split(), join() and grep() make
  working with QStringLists easy. 
*/

/*!
 \fn QStringList::QStringList()
  Creates an empty list.
*/

/*! \fn QStringList::QStringList( const QStringList& l )

  Creates a copy of the list \a l. This function is very fast because
  QStringList is implicitly shared. However, for the programmer this
  is the same as a deep copy. If this list or the original one or some
  other list referencing the same shared data is modified, the
  modifying list first makes a copy.
*/

/*!
  \fn QStringList::QStringList (const QString & i)

  Constructs a string list consisting of the single string \a i.
  Longer lists are easily created as follows:

  \walkthrough table/small-table-demo/main.cpp
  \skipto QStringList
  \printline QStringList
  \printline comboEntries << 

  (example code taken from \link small-table-demo-example.html
  table/small-table-demo/main.cpp\endlink)
*/

/*!
  \fn QStringList::QStringList (const char* i)
  Constructs a string list consisting of the single latin-1 string \a i.
*/

/*! \fn QStringList::QStringList( const QValueList<QString>& l )

  Constructs a new string list that is a copy of \a l.
*/

/*!
  Sorts the list of strings in ascending order.

  Sorting is very fast. It uses the Qt Template Library's
  efficient HeapSort implementation that operates in O(n*log n).
*/
void QStringList::sort()
{
    qHeapSort( *this );
}

/*! \overload

  This version of the function uses a QChar as separator, rather than
  a full regular expression.

  \sa join()
*/

QStringList QStringList::split( const QChar &sep, const QString &str,
				bool allowEmptyEntries )
{
    return split( QString(sep), str, allowEmptyEntries );
}

/*! \overload

  This version of the function uses a QChar as separator, rather than
  a full regular expression.

  If \a sep is an empty string, the return value is a list of
  one-character strings: split( QString( "" ), "mfc" ) returns the
  three-item list "m", "f", "c".

  If \a allowEmptyEntries is TRUE, an empty string is inserted in the
  list wherever the separator matches twice without intervening
  text.
*/

QStringList QStringList::split( const QString &sep, const QString &str,
				bool allowEmptyEntries )
{
    QStringList lst;

    int j = 0;
    int i = str.find( sep, j );

    while ( i != -1 ) {
	if ( i > j && i <= (int)str.length() )
	    lst << str.mid( j, i - j );
	else if ( allowEmptyEntries )
	    lst << QString::null;
	j = i + sep.length();
	i = str.find( sep, sep.length() > 0 ? j : j+1 );
    }

    int l = str.length() - 1;
    if ( str.mid( j, l - j + 1 ).length() > 0 )
	lst << str.mid( j, l - j + 1 );
    else if ( allowEmptyEntries )
	lst << QString::null;

    return lst;
}

/*! Splits the string \a str into strings where \a sep occurs, and
  returns the list of those strings.

  If \a allowEmptyEntries is TRUE, an empty string is inserted in the
  list wherever the separator matches twice without intervening
  text.

  For example, if you split the string "a,,b,c" on commas, split()
  returns the three-item list "a", "b", "c" if \a allowEmptyEntries is
  FALSE (the default), and the four-item list "a", "", "b", "c" if \a
  allowEmptyEntries is TRUE.

  If \a sep does not match anywhere in \a str, split() returns a list
  consisting of the single string \a str.

  \sa join()
*/

QStringList QStringList::split( const QRegExp &sep, const QString &str,
				bool allowEmptyEntries )
{
    QStringList lst;

    QRegExp tep = sep;

    int j = 0;
    int i = tep.search( str, j );

    while ( i != -1 ) {
	if ( str.mid( j, i - j ).length() > 0 )
	    lst << str.mid( j, i - j );
	else if ( allowEmptyEntries )
	    lst << QString::null;
	if ( tep.matchedLength() == 0 )
	    j = i + 1;
	else
	    j = i + tep.matchedLength();
	i = tep.search( str, j );
    }

    int l = str.length() - 1;
    if ( str.mid( j, l - j + 1 ).length() > 0 )
	lst << str.mid( j, l - j + 1 );
    else if ( allowEmptyEntries )
	lst << QString::null;

    return lst;
}

/*!
  Returns a list of all strings containing the substring \a str.

  If \a cs is TRUE, the grep is done case-sensitively, otherwise not.
*/

QStringList QStringList::grep( const QString &str, bool cs ) const
{
    QStringList res;
    for ( QStringList::ConstIterator it = begin(); it != end(); ++it )
	if ( (*it).contains(str, cs) )
	    res << *it;

    return res;
}

/*! \overload

  Returns a list of all strings containing a substring that matches
  the regular expression \a expr.
*/

QStringList QStringList::grep( const QRegExp &expr ) const
{
    QStringList res;
    for ( QStringList::ConstIterator it = begin(); it != end(); ++it )
	if ( (*it).contains(expr) )
	    res << *it;

    return res;
}

/*!
  Joins the stringlist into a single string with each element
  separated by \a sep.

  \sa split()
*/
QString QStringList::join( const QString &sep ) const
{
    QString res;
    bool alredy = FALSE;
    for ( QStringList::ConstIterator it = begin(); it != end(); ++it ) {
	if ( alredy )
	    res += sep;
	alredy = TRUE;
	res += *it;
    }

    return res;
}

#ifndef QT_NO_DATASTREAM
Q_EXPORT QDataStream &operator>>( QDataStream & s, QStringList& l )
{
    return s >> (QValueList<QString>&)l;
}

Q_EXPORT QDataStream &operator<<( QDataStream & s, const QStringList& l )
{
    return s << (const QValueList<QString>&)l;
}
#endif

/*! Converts from a ASCII-QStrList \a ascii to a QStringList (Unicode).
*/
QStringList QStringList::fromStrList(const QStrList& ascii)
{
    QStringList res;
    const char * s;
    for ( QStrListIterator it(ascii); (s=it.current()); ++it )
	res << s;
    return res;
}

#endif //QT_NO_STRINGLIST
