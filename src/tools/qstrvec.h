/****************************************************************************
** $Id: //depot/qt/main/src/tools/qstrvec.h#4 $
**
** Definition of QStrVec and QStrIVec classes
**
** Author  : Haavard Nord
** Created : 931203
**
** Copyright (C) 1993-1995 by Troll Tech AS.  All rights reserved.
**
** --------------------------------------------------------------------------
** The QStrVector class provides a vector of strings (i.e., char *).
*****************************************************************************/

#ifndef QSTRVEC_H
#define QSTRVEC_H

#include "qstring.h"
#include "qvector.h"
#include "qdstream.h"


declare(QVectorM,char);


class QStrVec : public QVectorM(char)		// string vector
{
public:
    QStrVec()  { dc = TRUE; }
    QStrVec( uint size, bool deepc = TRUE ) : QVectorM(char)(size) {dc=deepc;}
   ~QStrVec()  { clear(); }
private:
    GCI	 newItem( GCI d )	{ return dc ? qstrdup( (char*)d ) : d; }
    void deleteItem( GCI d )	{ if ( dc ) delete[] (char*)d; }
    int	 compareItems( GCI s1, GCI s2 )
				{ return strcmp((char*)s1,(char*)s2); }
    QDataStream &read( QDataStream &s, GCI &d )
				{ s >> (char *&)d; return s; }
    QDataStream &write( QDataStream &s, GCI d ) const
				{ return s << (char *)d; }
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
				{ return stricmp((char*)s1,(char*)s2); }
};


#endif // QSTRVEC_H
