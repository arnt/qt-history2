/*
  openedlist.h
*/

#ifndef OPENEDLIST_H
#define OPENEDLIST_H

#include <qstring.h>

#include "location.h"

class OpenedList
{
public:
    enum Style { Bullet, Tag, Value, Numeric, UpperAlpha, LowerAlpha,
		 UpperRoman, LowerRoman };

    OpenedList()
	: sty( Bullet ), ini( 1 ), nex( 0 ) { }
    OpenedList( Style style );
    OpenedList( const Location& location, const QString& hint );

    void next() { nex++; }

    bool isStarted() const { return nex >= ini; }
    Style style() const { return sty; }
    QString styleString() const;
    int number() const { return nex; }
    QString numberString() const;
    QString prefix() const { return pref; }
    QString suffix() const { return suff; }

private:
    static QString toAlpha( int n );
    static int fromAlpha( const QString& str );
    static QString toRoman( int n );
    static int fromRoman( const QString& str );

    Style sty;
    int ini;
    int nex;
    QString pref;
    QString suff;
};

#endif
