/****************************************************************************
** $Id$
**
** Implementation of QSettings class
**
** Created: 2000.06.26
**
** Copyright (C) 2000-2001 Trolltech AS.  All rights reserved.
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

#include "qsettings.h"
#include "qregexp.h"

#ifndef QT_NO_SETTINGS

/*!
    \overload
    Writes the string list entry \a value into key \a key. The \a key is
    created if it doesn't exist. Any previous value is overwritten by \a
    value. The list is stored as a sequence of strings separated by \a
    separator, so none of the strings in the list should contain the
    separator. If the list is empty or null the key's value will be an
    empty string.

    If an error occurs the settings are left unchanged and FALSE is
    returned; otherwise TRUE is returned.

  \sa readListEntry(), readNumEntry(), readDoubleEntry(), readBoolEntry(), removeEntry()
*/
bool QSettings::writeEntry(const QString &key, const QStringList &value,
			   const QChar &separator)
{
    QString s(value.join(separator));
    return writeEntry(key, s);
}

/*!
    \overload
    Writes the string list entry \a value into key \a key. The \a key is
    created if it doesn't exist. Any previous value is overwritten by \a
    value.

    If an error occurs the settings are left unchanged and FALSE is
    returned; otherwise TRUE is returned.

  \sa readListEntry(), readNumEntry(), readDoubleEntry(), readBoolEntry(), removeEntry()
*/
bool QSettings::writeEntry(const QString &key, const QStringList &value)
{
    QString s;
    for (QStringList::ConstIterator it=value.begin(); it!=value.end(); ++it) {
	QString el = *it;
	if ( el.isNull() ) {
	    el = "^0";
	} else {
	    el.replace(QRegExp("\\^"), "^^");
	}
	s+=el;
	s+="^e"; // end of element
    }
    return writeEntry(key, s);
}


/*!
    \overload

  Reads the entry specified by \a key as a string.  The \a separator is
  used to create a QStringList by calling QStringList::split(\a
  separator, entry).
  If \a ok is non-null, *ok is set to TRUE if the key was read, FALSE
  otherwise.

  Note that if you want to iterate over the list, you should
  iterate over a copy, e.g.
    \code
    QStringList list = mySettings.readListEntry( "size", " " );
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

  \sa readEntry(), readDoubleEntry(), readBoolEntry(), writeEntry(), removeEntry(), QStringList::split()
*/
QStringList QSettings::readListEntry(const QString &key, const QChar &separator, bool *ok )
{
    QString value = readEntry( key, QString::null, ok );
    if ( ok && !*ok )
	return QStringList();

    return QStringList::split(separator, value);
}

/*!
  Reads the entry specified by \a key as a string.
  If \a ok is non-null, *ok is set to TRUE if the key was read, FALSE
  otherwise.

  Note that if you want to iterate over the list, you should
  iterate over a copy, e.g.
    \code
    QStringList list = mySettings.readListEntry( "recentfiles" );
    QStringList::Iterator it = list.begin();
    while( it != list.end() ) {
	myProcessing( *it );
	++it;
    }
    \endcode

  \sa readEntry(), readDoubleEntry(), readBoolEntry(), writeEntry(), removeEntry(), QStringList::split()
*/
QStringList QSettings::readListEntry(const QString &key, bool *ok )
{
    QString value = readEntry( key, QString::null, ok );
    if ( ok && !*ok )
	return QStringList();
    QStringList l;
    QString s;
    bool esc=FALSE;
    for (int i=0; i<(int)value.length(); i++) {
	if ( esc ) {
	    if ( value[i] == 'e' ) { // end-of-string
		l.append(s);
		s="";
	    } else if ( value[i] == '0' ) { // null string
		s=QString::null;
	    } else {
		s.append(value[i]);
	    }
	    esc=FALSE;
	} else if ( value[i] == '^' ) {
	    esc = TRUE;
	} else {
	    s.append(value[i]);
	}
    }
    return l;
}

#endif
