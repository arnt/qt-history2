/*
  location.h
*/

#ifndef LOCATION_H
#define LOCATION_H

#include <qstring.h>

/*
  The Location class represents a point in a file. It contains the
  file path, line line number, and column number. Numbering starts at
  1, even for columns. However, The column number is unused so far in
  qdoc.
*/
class Location
{
public:
    static Location fromString( const QString& str );

    Location()
	: fp(), ln( 0 ), cn( 0 ) { }
    Location( const QString& filePath )
	: fp( filePath ), ln( 1 ), cn( 1 ) { }
    Location( const Location& loc )
	: fp( loc.fp ), ln( loc.ln ), cn( loc.cn ) { }

    Location& operator=( const Location& loc );

    void advance( QChar ch );

    QString fileName() const;
    const QString& filePath() const { return fp; }
    QString shortFilePath() const;
    int lineNum() const { return ln; }
    int columnNum() const { return cn; }

    QString toString() const;

private:
    QString fp;
    int ln;
    int cn;
};

#endif
