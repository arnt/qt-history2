/*
  location.h
*/

#ifndef LOCATION_H
#define LOCATION_H

#include <qstring.h>

/*
  The class Location represents a point in a file.
*/
class Location
{
public:
    Location()
	: fp(), ln( 0 ), cn( 0 ) { }
    Location( const QString& filePath )
	: fp( filePath ), ln( 1 ), cn( 1 ) { }
    Location( const Location& loc )
	: fp( loc.fp ), ln( loc.ln ), cn( loc.cn ) { }

    Location& operator=( const Location& loc );

    void advance( int ch );

    QString fileName() const;
    const QString& filePath() const { return fp; }
    QString shortFilePath() const;
    int lineNum() const { return ln; }
    int columnNum() const { return cn; }

private:
    QString fp;
    int ln;
    int cn;
};

#endif
