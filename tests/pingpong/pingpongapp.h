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
class QListView;
class QAction;
class QTabWidget;

class TeamEditorWidget;

class MatchTable : public QSqlTable
{
public:
    MatchTable( QWidget * parent = 0, const char * name = 0 );
    void         sortColumn ( int col, bool ascending = TRUE,
			      bool wholeRows = FALSE );
};

class Statistics : public QFrame
{
public:
    Statistics( QWidget * parent = 0, const char * name = 0 );
    void refresh();
private:
    QListView* list;
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
    void updateIcons( QWidget * );
    void editTeams();
    
private:
    QAction * insertResultAc;
    QAction * updateResultAc;
    QAction * deleteResultAc;
    
    QTabWidget * tab;
    TeamEditorWidget * teamEditor;
    QSqlTable * matchTable;
    MatchCursor matchCr;
};

#endif

