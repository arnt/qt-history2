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
    Walkthrough() : shutUp( FALSE ), justIncluded( TRUE ) { }

    QString include( const QString& fileName, const Resolver *resolver = 0 );
    void start( const QString& fileName, const Resolver *resolver = 0 );

    QString printline( const QString& substr, const Location& docLoc );
    QString printto( const QString& substr, const Location& docLoc );
    QString printuntil( const QString& substr, const Location& docLoc );
    void skipline( const QString& substr, const Location& docLoc );
    void skipto( const QString& substr, const Location& docLoc );
    void skipuntil( const QString& substr, const Location& docLoc );

    const QString& filePath() const { return fpath; }

private:
#if defined(QT_DISABLE_COPY)
    Walkthrough( const Walkthrough& w );
    Walkthrough& operator=( const Walkthrough& w );
#endif

    QString start( bool localLinks, const QString& fileName,
		   const Resolver *resolver );

    QString xline( const QString& substr, const Location& docLoc,
		   const QString& command );
    QString xto( const QString& substr, const Location& docLoc,
		   const QString& command );
    QString xuntil( const QString& substr, const Location& docLoc,
		   const QString& command );
    QString getNextLine( const Location& docLoc );

    QString fpath;
    QString fancyText;
    QStringList plainlines;
    QStringList fancylines;
    Location walkloc;
    bool shutUp;
    bool justIncluded;
};

#endif
