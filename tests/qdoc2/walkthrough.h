/*
  walkthrough.h
*/

#ifndef WALKTHROUGH_H
#define WALKTHROUGH_H

#include <qstring.h>
#include <qstringlist.h>

#include "location.h"

class Resolver;

/*
  The Walkthrough class implements the C++ example walkthrough engine.

  In the qdoc comments, '\include' or '\dontinclude' starts the walkthrough
  engine.  Afterwards, special commands are sent to the engine.  These commands
  are

      \printline
      \printto
      \printuntil
      \skipline
      \skipto
      \skipuntil

  and they invoke the method of the same name.
*/
class Walkthrough
{
public:
    Walkthrough() { }
    Walkthrough( const Walkthrough& w );

    Walkthrough& operator=( const Walkthrough& w );

    QString start( const QString& filePath, const Resolver *resolver = 0 );
    void dontstart( const QString& filePath, const Resolver *resolver = 0 );

    QString printline( const QString& substr, const Location& docLoc );
    QString printto( const QString& substr, const Location& docLoc );
    QString printuntil( const QString& substr, const Location& docLoc );
    void skipline( const QString& substr, const Location& docLoc );
    void skipto( const QString& substr, const Location& docLoc );
    void skipuntil( const QString& substr, const Location& docLoc );

private:
    QString start( bool localLinks, const QString& filePath,
		   const Resolver *resolver );

    QString xline( const QString& substr, const Location& docLoc,
		   const QString& command );
    QString xto( const QString& substr, const Location& docLoc,
		   const QString& command );
    QString xuntil( const QString& substr, const Location& docLoc,
		   const QString& command );
    void skipEmptyLines();
    QString getNextLine();

    QStringList plainlines;
    QStringList processedlines;
    Location walkloc;
};

#endif
