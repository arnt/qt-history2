/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstrvec.h#20 $
**
** Definition of QStrVec and QStrIVec classes
**
** Created : 931203
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

#ifndef QSTRVEC_H
#define QSTRVEC_H

#ifndef QT_H
#include "qstring.h"
#include "qvector.h"
#include "qdatastream.h"
#endif // QT_H


#if defined(Q_TEMPLATEDLL)
template class Q_EXPORT QVector<char>
#endif

typedef QVector<char> QStrVecBase;


class Q_EXPORT QStrVec : public QStrVecBase
{
public:
    QStrVec()  { dc = TRUE; }
    QStrVec( uint size, bool deepc = TRUE ) : QStrVecBase(size) {dc=deepc;}
   ~QStrVec()  { clear(); }
private:
    Item	 newItem( Item d )	{ return dc ? qstrdup( (const char*)d ) : d; }
    void deleteItem( Item d )	{ if ( dc ) delete[] (char*)d; }
    int	 compareItems( Item s1, Item s2 )
				{ return strcmp((const char*)s1,
						(const char*)s2); }
    QDataStream &read( QDataStream &s, Item &d )
				{ s >> (char *&)d; return s; }
    QDataStream &write( QDataStream &s, Item d ) const
				{ return s << (const char*)d; }
    bool dc;
};


class Q_EXPORT QStrIVec : public QStrVec	// case insensitive string vec
{
public:
    QStrIVec() {}
    QStrIVec( uint size, bool dc = TRUE ) : QStrVec( size, dc ) {}
   ~QStrIVec() { clear(); }
private:
    int	 compareItems( Item s1, Item s2 )
				{ return stricmp((const char*)s1,
						 (const char*)s2); }
};


#endif // QSTRVEC_H
