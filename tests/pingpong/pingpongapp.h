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

class MatchCursor : public QSqlCursor
{
public:
    MatchCursor();

protected:
    QVariant calculateField( const QString& name );
    void     primeInsert( QSqlRecord* buf );

    QSqlCursor * teamCr;
};

class PlayerCursor : public QSqlCursor
{
public:
    PlayerCursor();
protected:
    void     primeInsert( QSqlRecord* buf );
};

class TeamCursor : public QSqlCursor
{
public:
    TeamCursor();
protected:
    void     primeInsert( QSqlRecord* buf );
};

class MatchTable : public QSqlTable
{
public:
    MatchTable( QWidget * parent = 0, const char * name = 0 );
    void         sortColumn ( int col, bool ascending = TRUE,
			      bool wholeRows = FALSE );
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

    void editPlayer();
    void editTeam();

protected:
    void editWindow( QSqlCursor& cursor, const QString& sortField, const QString& caption );

private:
    QSqlTable * matchTable;
    MatchCursor matchCr;
};

#endif

