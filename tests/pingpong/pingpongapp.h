#ifndef PINGPONGAPP_H
#define PINGPONGAPP_H

#include <qmainwindow.h>
#include <qsqltable.h>

#include "cursors.h"

class QLabel;
class QListView;
class QAction;
class QTabWidget;

class TeamEditorWidget;
class TeamPicker;
class Statistics;

class Statistics : public QWidget
{
    Q_OBJECT
public:
    Statistics( QWidget * parent = 0, const char * name = 0 );

protected slots:
    void updateStats();

private:
    TeamCursor teamCr;
    QLabel * setsWon;
    QLabel * setsLost;
    QLabel * matchesWon;
    QLabel * matchesLost;
    QLabel * winPercentage;
    QLabel * lossPercentage;
    QLabel * totalSets;
    QLabel * totalMatches;
    QLabel * hate;
    QLabel * love;
    QLabel * topTeam;
    TeamPicker * teamPicker;
};

class HallOfFame : public QWidget
{
public:
    HallOfFame( QWidget * parent = 0, const char * name = 0 );

private:
    QListView * scorelist;
    TeamCursor teamCr;
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
    Statistics * statWidget;
    HallOfFame * hallOfFameWidget;
    QSqlTable * matchTable;
    MatchView matchView;
    MatchCursor matchCursor;
};

#endif

