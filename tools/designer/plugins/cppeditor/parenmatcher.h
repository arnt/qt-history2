#ifndef PARENMATCHER_H
#define PARENMATCHER_H

#include <qstring.h>
#include <qvaluelist.h>

class QTextCursor;

struct Paren 
{
    Paren() : type( Open ), chr( ' ' ), pos( -1 ) {}
    Paren( int t, const QChar &c, int p ) : type( (Type)t ), chr( c ), pos( p ) {}
    enum Type { Open, Closed };
    Type type;
    QChar chr;
    int pos;
};

typedef QValueList<Paren> ParenList;

class ParenMatcher
{
public:
    ParenMatcher();
    
    virtual bool match( QTextCursor *c );
    
private:
    bool checkOpenParen( QTextCursor *c );
    bool checkClosedParen( QTextCursor *c );
    
};

#endif
