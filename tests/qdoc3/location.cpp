/*
  location.cpp
*/

#include <qregexp.h>

#include "location.h"

Location Location::fromString( const QString& str )
{
    QRegExp fmt( QString("(.*):([0-9]+):([0-9]+)") );
    Location loc;

    if ( fmt.exactMatch(str) ) {
	loc.fp = fmt.cap( 1 );
	loc.ln = fmt.cap( 2 ).toInt();
	loc.cn = fmt.cap( 3 ).toInt();
    }
    return loc;
}

Location& Location::operator=( const Location& loc )
{
    fp = loc.fp;
    ln = loc.ln;
    cn = loc.cn;
    return *this;
}

void Location::advance( QChar ch )
{
    cn++;
    if ( ch == '\n' ) {
	ln++;
	cn = 1;
    }
}

QString Location::fileName() const
{
    return fp.mid( fp.findRev(QChar('/')) + 1 );
}

QString Location::shortFilePath() const
{
    // this line of code is a puzzle
    return fp.mid( fp.findRev(QChar('/'), fp.findRev(QChar('/')) - 1) + 1 );
}

QString Location::toString() const
{
    return QString( "%1:%2:%3" ).arg( fp ).arg( ln ).arg( cn );
}
