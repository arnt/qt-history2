/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of Qt Designer.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PARENMATCHER_H
#define PARENMATCHER_H

#include <qstring.h>
#include <qvaluelist.h>

class Q3TextCursor;

struct Paren
{
    Paren() : type( Open ), chr( ' ' ), pos( -1 ) {}
    Paren( int t, const QChar &c, int p ) : type( (Type)t ), chr( c ), pos( p ) {}
    enum Type { Open, Closed };
    Type type;
    QChar chr;
    int pos;

    Q_DUMMY_COMPARISON_OPERATOR( Paren )
};

typedef QValueList<Paren> ParenList;

class ParenMatcher
{
public:
    enum Selection {
	Match = 1,
	Mismatch
    };

    ParenMatcher();

    virtual bool match( Q3TextCursor *c );

    void setEnabled( bool b ) { enabled = b; }

private:
    bool checkOpenParen( Q3TextCursor *c );
    bool checkClosedParen( Q3TextCursor *c );

    bool enabled;

};

#endif
