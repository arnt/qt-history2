/*
  quoter.h
*/

#ifndef QUOTER_H
#define QUOTER_H

#include <qregexp.h>

#include "location.h"

class Quoter
{
public:
    Quoter();

    void reset();
    void quoteFromFile( const QString& filePath, const QString& plainCode,
			const QString& markedCode );
    QString quoteLine( const Location& docLocation, const QString& command,
		       const QString& pattern );
    QString quoteTo( const Location& docLocation, const QString& command,
		     const QString& pattern = "" );
    QString quoteUntil( const Location& docLocation, const QString& command,
			const QString& pattern = "" );

private:
    QString getLine();
    void failedAtEnd( const Location& docLocation, const QString& command );
    bool match( const Location& docLocation, const QString& pattern,
    		const QString& line );
    static QString fix( const QString& str );
    static QString trimWhiteSpace( const QString& str );

    bool verbose;
    bool validRegExp;
    QStringList plainLines;
    QStringList markedLines;
    Location codeLocation;
    QRegExp splitPoint;
    QRegExp manyEndls;
};

#endif
