/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstrvec.h#11 $
**
** Definition of QStrVec and QStrIVec classes
**
** Created : 931203
**
** Copyright (C) 1993-1997 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** The QStrVector class provides a vector of strings (i.e., char *).
*****************************************************************************/

#ifndef QSTRVEC_H
#define QSTRVEC_H

#include "qstring.h"
#include "qvector.h"
#include "qdstream.h"


#if defined(DEFAULT_TEMPLATECLASS)
typedef QVectorT<char>			QStrVecBase;
#else
typedef Q_DECLARE(QVectorM,char)		QStrVecBase;
#endif


class QStrVec : public QStrVecBase
{
public:
    QStrVec()  { dc = TRUE; }
    QStrVec( uint size, bool deepc = TRUE ) : QStrVecBase(size) {dc=deepc;}
   ~QStrVec()  { clear(); }
private:
    GCI	 newItem( GCI d )	{ return dc ? qstrdup( (const char*)d ) : d; }
    void deleteItem( GCI d )	{ if ( dc ) delete[] (char*)d; }
    int	 compareItems( GCI s1, GCI s2 )
				{ return strcmp((const char*)s1,
						(const char*)s2); }
    QDataStream &read( QDataStream &s, GCI &d )
				{ s >> (char *&)d; return s; }
    QDataStream &write( QDataStream &s, GCI d ) const
				{ return s << (const char *)d; }
    bool dc;
};


class QStrIVec : public QStrVec			// case insensitive string vec
{
public:
    QStrIVec() {}
    QStrIVec( uint size, bool dc = TRUE ) : QStrVec( size, dc ) {}
   ~QStrIVec() { clear(); }
private:
    int	 compareItems( GCI s1, GCI s2 )
				{ return stricmp((const char*)s1,
						 (const char*)s2); }
};


#endif // QSTRVEC_H
