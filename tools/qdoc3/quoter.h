/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
  quoter.h
*/

#ifndef QUOTER_H
#define QUOTER_H

#include <qregexp.h>
#include <qstringlist.h>

#include "location.h"

class Quoter
{
public:
    Quoter();

    void reset();
    void quoteFromFile( const QString& userFriendlyFileName,
			const QString& plainCode, const QString& markedCode );
    QString quoteLine( const Location& docLocation, const QString& command,
		       const QString& pattern );
    QString quoteTo( const Location& docLocation, const QString& command,
		     const QString& pattern);
    QString quoteUntil( const Location& docLocation, const QString& command,
			const QString& pattern);

private:
    QString getLine();
    void failedAtEnd( const Location& docLocation, const QString& command );
    bool match( const Location& docLocation, const QString& pattern,
    		const QString& line );
    static QString fix( const QString& str );
    static QString trimWhiteSpace( const QString& str );

    bool silent; 
    bool validRegExp;
    QStringList plainLines;
    QStringList markedLines;
    Location codeLocation;
    QRegExp splitPoint;
    QRegExp manyEndls;
};

#endif
