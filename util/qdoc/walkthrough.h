/*
  walkthrough.h
*/

#ifndef WALKTHROUGH_H
#define WALKTHROUGH_H

#include <qstring.h>
#include <qstringlist.h>

#include "codeprocessor.h"
#include "location.h"

class Resolver;

class HighScore
{
public:
    HighScore() : ininc( TRUE ), ln( 0 ), contri( 0 ), tota( 0 ) { }
    HighScore( const HighScore& hs )
	    : ininc( hs.ininc ), ln( hs.ln ), contri( hs.contri ),
	      tota( hs.tota ) { }

    HighScore& operator=( const HighScore& hs );

    void addContribution( bool inInclude, int lineNo, int contribution );

    bool inInclude() const { return ininc; }
    int lineNum() const { return ln; }
    int contribution() const { return contri; }
    int total() const { return tota; }

private:
    bool ininc;
    int ln;
    int contri;
    int tota;
};

// QMap<link, HighScore>
typedef QMap<QString, HighScore> ScoreMap;

// QMap<lineNumber, link set>
typedef QMap<int, StringSet> LinkMap;

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
    Walkthrough()
	: totalOccCount( 0 ), shutUp( FALSE ), justIncluded( TRUE ) { }

    void includePass1( const QString& fileName, const Resolver *resolver );
    QString includePass2( const QString& fileName, const Resolver *resolver,
			  const LinkMap& includeLinkMap,
			  const LinkMap& walkthroughLinkMap );
    void startPass1( const QString& fileName, const Resolver *resolver );
    void startPass2( const QString& fileName, const Resolver *resolver,
		     const LinkMap& walkthroughLinkMap );

    QString printline( const QString& substr, const Location& docLoc );
    QString printto( const QString& substr, const Location& docLoc );
    QString printuntil( const QString& substr, const Location& docLoc );
    void skipline( const QString& substr, const Location& docLoc );
    void skipto( const QString& substr, const Location& docLoc );
    void skipuntil( const QString& substr, const Location& docLoc );

    const QString& fileName() const { return fname; }
    const QString& filePath() const { return fpath; }
    const ScoreMap& scoreMap() const { return scores; }

private:
#if defined(QT_DISABLE_COPY)
    Walkthrough( const Walkthrough& w );
    Walkthrough& operator=( const Walkthrough& w );
#endif

    void addANames( QString *text, const LinkMap& exampleLinkMap );
    QString start( bool include, bool firstPass, const QString& fileName,
		   const Resolver *resolver, const LinkMap& includeLinkMap,
		   const LinkMap& walkthroughLinkMap );

    QString xline( const QString& substr, const Location& docLoc,
		   const QString& command );
    QString xto( const QString& substr, const Location& docLoc,
		 const QString& command );
    QString xuntil( const QString& substr, const Location& docLoc,
		    const QString& command );
    QString getNextLine( const Location& docLoc );
    void incrementScores( bool include, int lineNo, int contribution );

    QString fname;
    QString fpath;
    OccurrenceMap occMap;
    QMap<QString, int> classOccCounts;
    int totalOccCount;
    ScoreMap scores;
    LinkMap exmap;
    QString includeText;
    QStringList plainlines;
    QStringList fancylines;
    Location walkloc;
    bool shutUp;
    bool justIncluded;
};

#endif
