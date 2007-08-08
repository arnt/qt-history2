/*
  location.h
*/

#ifndef LOCATION_H
#define LOCATION_H

#include <qstack.h>

#include "tr.h"

class Config;
class QRegExp;

class Location
{
public:
    Location();
    Location( const QString& filePath );
    Location( const Location& other );
    ~Location() { delete stk; }

    Location& operator=( const Location& other );

    void start();
    void advance( QChar ch );
    void push( const QString& filePath );
    void pop();
    void setEtc( bool etc ) { etcetera = etc; }
    void setLineNo(int no) { stkTop->lineNo = no; }
    void setColumnNo(int no) { stkTop->columnNo = no; }

    bool isEmpty() const { return stkDepth == 0; }
    int depth() const { return stkDepth; }
    const QString& filePath() const { return stkTop->filePath; }
    QString fileName() const;
    int lineNo() const { return stkTop->lineNo; }
    int columnNo() const { return stkTop->columnNo; }
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
    enum MessageType { Warning, Error };

    struct StackEntry
    {
	QString filePath;
	int lineNo;
	int columnNo;
    };

    void emitMessage( MessageType type, const QString& message,
		      const QString& details ) const;
    QString toString() const;
    QString top() const;

    StackEntry stkBottom;
    QStack<StackEntry> *stk;
    StackEntry *stkTop;
    int stkDepth;
    bool etcetera;

    static int tabSize;
    static QString programName;
    static QRegExp *spuriousRegExp;
};

#endif
