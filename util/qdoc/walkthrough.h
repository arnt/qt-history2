/*
  walkthrough.h
*/

#ifndef WALKTHROUGH_H
#define WALKTHROUGH_H

#include <qstring.h>
#include <qstringlist.h>

#include "location.h"

class Resolver;

class LineScore
{
public:
    LineScore() : lin( 0 ), scor( 0 ) { }
    LineScore( int line, int score ) : lin( line ), scor( score ) { }
    LineScore( const LineScore& ls ) : lin( ls.lin ), scor( ls.scor ) { }

    LineScore& operator=( const LineScore& ls ) {
	lin = ls.lin;
	scor = ls.scor;
	return *this;
    }

    int line() const { return lin; }
    int score() const { return scor; }

private:
    int lin;
    int scor;
};

typedef QMap<QString, LineScore> ScoreMap;

/*
  The Walkthrough class implements the C++ example walkthrough engine.

  In the qdoc comments, '\walkthrough' starts the walkthrough engine.
  Afterwards, special commands are sent to the engine. These commands
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

    void includePass1( const QString& fileName, const Resolver *resolver );
    QString includePass2( const QString& fileName, const Resolver *resolver );
    void startPass1( const QString& fileName, const Resolver *resolver );
    void startPass2( const QString& fileName, const Resolver *resolver );

    QString printline( const QString& substr, const Location& docLoc );
    QString printto( const QString& substr, const Location& docLoc );
    QString printuntil( const QString& substr, const Location& docLoc );
    void skipline( const QString& substr, const Location& docLoc );
    void skipto( const QString& substr, const Location& docLoc );
    void skipuntil( const QString& substr, const Location& docLoc );

    const QString& filePath() const { return fpath; }
    const ScoreMap& scoreMap() const { return scores; }

private:
#if defined(QT_DISABLE_COPY)
    Walkthrough( const Walkthrough& w );
    Walkthrough& operator=( const Walkthrough& w );
#endif

    QString start( bool include, bool firstPass, const QString& fileName,
		   const Resolver *resolver );

    QString xline( const QString& substr, const Location& docLoc,
		   const QString& command );
    QString xto( const QString& substr, const Location& docLoc,
		 const QString& command );
    QString xuntil( const QString& substr, const Location& docLoc,
		    const QString& command );
    QString getNextLine( const Location& docLoc );

    QString fpath;
    OccurrenceMap occMap;
    ScoreMap scores;
    QString fancyText;
    QStringList plainlines;
    QStringList fancylines;
    Location walkloc;
    bool shutUp;
    bool justIncluded;
};

#endif
