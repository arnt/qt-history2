/*
  location.cpp
*/

#include "location.h"

Location& Location::operator=( const Location& loc )
{
    fp = loc.fp;
    ln = loc.ln;
    cn = loc.cn;
    return *this;
}

void Location::advance( int ch )
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
