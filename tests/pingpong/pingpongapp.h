#ifndef PINGPONGAPP_H
#define PINGPONGAPP_H

#include <qmainwindow.h>
#include <qsqltable.h>

#include "cursors.h"

class QLabel;
class QListView;
class QAction;
class QTabWidget;

class TeamPicker;
class TeamEditor;
class Statistics;
class HighscoreList;

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

    QTabWidget    * tab;
    TeamEditor    * teamEditor;
    Statistics    * statistics;
    HighscoreList * highscore;
    QSqlTable     * matchTable;
    MatchView     matchView;
    MatchCursor   matchCursor;
};

#endif

