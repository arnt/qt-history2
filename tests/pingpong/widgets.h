#ifndef WIDGETS_H
#define WIDGETS_H

#include <qcombobox.h>
#include <qmap.h>
#include "cursors.h"

class QSqlTable;
class QLabel;
class QListView;

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

class TeamEditor : public QWidget
{
    Q_OBJECT

public:
    TeamEditor( QWidget * parent = 0, const char * name = 0 );
    
protected slots:
    void updateTeamMembers( const QSqlRecord * record );
    void addPlayer();
    void removePlayer();

private:
    QSqlTable * teamTable;
    QSqlTable * playerTable;
    QSqlTable * player2teamTable;

    QLabel * player2teamLabel;
    Player2TeamView player2teamView;
    Player2TeamCursor player2teamCursor;
    TeamCursor   teamCursor;
    PlayerCursor playerCursor;
};

class Statistics : public QWidget
{
    Q_OBJECT
public:
    Statistics( QWidget * parent = 0, const char * name = 0 );

public slots:
    void update();

private:
    TeamPicker * teamPicker;
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
};

class HighscoreList : public QWidget
{
    Q_OBJECT
public:
    HighscoreList( QWidget * parent = 0, const char * name = 0 );

public slots:
    void update();
    
private:
    QListView * list;
};

#endif // WIDGETS_H
