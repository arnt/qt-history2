/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3STRVEC_H
#define Q3STRVEC_H

#include "qstring.h"
#include "q3ptrvector.h"
#include "qdatastream.h"

class Q_COMPAT_EXPORT Q3StrVec : public Q3PtrVector<char>
{
public:
    Q3StrVec()  { dc = true; }
    Q3StrVec( uint size, bool deepc = true ) : Q3PtrVector<char>(size) {dc=deepc;}
   ~Q3StrVec()  { clear(); }
private:
    Item	 newItem( Item d )	{ return dc ? qstrdup( (const char*)d ) : d; }
    void deleteItem( Item d )	{ if ( dc ) delete[] (char*)d; }
    int	 compareItems( Item s1, Item s2 )
				{ return qstrcmp((const char*)s1,
						(const char*)s2); }
#ifndef QT_NO_DATASTREAM
    QDataStream &read( QDataStream &s, Item &d )
				{ s >> (char *&)d; return s; }
    QDataStream &write( QDataStream &s, Item d ) const
				{ return s << (const char*)d; }
#endif
    bool dc;
};


class Q_COMPAT_EXPORT Q3StrIVec : public Q3StrVec	// case insensitive string vec
{
public:
    Q3StrIVec() {}
    Q3StrIVec( uint size, bool dc = true ) : Q3StrVec( size, dc ) {}
   ~Q3StrIVec() { clear(); }
private:
    int	 compareItems( Item s1, Item s2 )
				{ return qstricmp((const char*)s1,
						 (const char*)s2); }
};


#endif // QSTRVEC_H
