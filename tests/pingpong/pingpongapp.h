#ifndef PINGPONGAPP_H
#define PINGPONGAPP_H

#include <qmainwindow.h>
#include <qcombobox.h>
#include <qmap.h>
#include <qsqlcursor.h>
#include <qsqltable.h>
#include <qsqleditorfactory.h>

#include "cursors.h"

class QLabel;

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

    void editTeams();

private:
    QSqlTable * matchTable;
    MatchCursor matchCr;
};

#endif

