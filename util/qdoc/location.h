/*
  location.h
*/

#ifndef LOCATION_H
#define LOCATION_H

#include <qstring.h>

/* The Location class represents a point in a file.  It contains the
  file path, line line number, and column number.  Numbering starts at
  1, even for columns.

  Incidentally, the column number is unused so far in qdoc.
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
