#ifndef PINGPONGAPP_H
#define PINGPONGAPP_H

#include <qmainwindow.h>
#include <qsqlcursor.h>
#include <qsqltable.h>

class QLabel;
class QSqlForm;

class MatchCursor : public QSqlCursor
{
public:
    MatchCursor();
    
protected:
    QVariant calculateField( uint fieldNumber );
    
    QSqlCursor * teamCr;
};

class PingPongApp : public QMainWindow
{
    Q_OBJECT
public:
    PingPongApp( QWidget * parent = 0, const char * name = 0 );

protected:
    void init();
    
protected slots:
    void updateMatch();
    void insertMatch();
    void deleteMatch();
    
private:
    QSqlTable * matchTable;
    MatchCursor matchCr;
};

#endif 

