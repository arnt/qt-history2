#ifndef PINGPONGFRONTEND_H
#define PINGPONGFRONTEND_H

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

class PingpongFrontEnd : public QMainWindow
{
    Q_OBJECT
public:
    PingpongFrontEnd( QWidget * parent = 0, const char * name = 0 );

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
    QWidget       * matchBase;
    QSqlTable     * matchTable;
    MatchView     matchView;
    MatchCursor   matchCursor;
};

#endif

