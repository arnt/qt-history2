#ifndef PINGPONGAPP_H
#define PINGPONGAPP_H

#include <qmainwindow.h>
#include <qcombobox.h>
#include <qmap.h>
#include <qsqlcursor.h>
#include <qsqltable.h>
#include <qsqleditorfactory.h>

class QLabel;
class QSqlForm;

class TeamPicker : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY( int teamid READ teamId WRITE setTeamId )
	
public:
    TeamPicker( QWidget * parent = 0, const char * name = 0 );    
    int teamId() const;
    void setTeamId( int id );
    
private:
    QMap< int, int > index2Id;
};

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

