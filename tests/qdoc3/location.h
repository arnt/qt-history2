/*
  location.h
*/

#ifndef LOCATION_H
#define LOCATION_H

#include <qvaluestack.h>

#include "tr.h"

class Config;
class QRegExp;

class Location
{
public:
    Location();
    Location( const QString& pathAndFileName );

    void start();
    void advance( QChar ch );
    void push( const QString& pathAndFileName );
    void pop();
    void setEtc( bool etc ) { etcetera = etc; }

    bool isEmpty() const { return stk.isEmpty(); }
    int depth() const { return stk.count(); }
    const QString& pathAndFileName() const { return stk.top().pathAndFileName; }
    QString fileName() const;
    int lineNo() const { return stk.top().lineNo; }
    int columnNo() const { return stk.top().columnNo; }
    bool etc() const { return etcetera; }
    void warning( const QString& message, const QString& details = "" ) const;
    void error( const QString& message, const QString& details = "" ) const;
    void fatal( const QString& message, const QString& details = "" ) const;

    QT_STATIC_CONST Location null;

    static void initialize( const Config& config );
    static void terminate();
    static void information( const QString& message );
    static void internalError( const QString& hint );

private:
    struct StackEntry
    {
	QString pathAndFileName;
	int lineNo;
	int columnNo;
    };

    void emitMessage( bool isWarning, const QString& message,
		      const QString& details ) const;
    QString toString() const;
    QString top() const;

    QValueStack<StackEntry> stk;
    bool etcetera;

    static int tabSize;
    static QString programName;
    static QRegExp *spuriousRegExp;
};

#endif
