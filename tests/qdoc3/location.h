/*
  location.h
*/

#ifndef LOCATION_H
#define LOCATION_H

#include <qvaluestack.h>

class Config;

class Location
{
public:
    Location();
    Location( const QString& pathAndFileName );

    void start();
    void advance( QChar ch );
    void push( const QString& pathAndFileName );
    void pop();

    bool isEmpty() const { return stk.isEmpty(); }
    const QString& pathAndFileName() const { return stk.top().pathAndFileName; }
    QString fileName() const;
    int lineNo() const { return stk.top().lineNo; }
    int columnNo() const { return stk.top().columnNo; }

    QT_STATIC_CONST Location null;

    static void initialize( const Config& config );
    static void terminate() { }

private:
    struct StackEntry
    {
	QString pathAndFileName;
	int lineNo;
	int columnNo;
    };

    QValueStack<StackEntry> stk;
};

#endif
