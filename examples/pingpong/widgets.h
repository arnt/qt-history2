#ifndef WIDGETS_H
#define WIDGETS_H

#include <qcombobox.h>
#include <qmap.h>
#include "cursors.h"
#include "teameditorbase.h"
#include "statisticsbase.h"

class QSqlTable;
class QLabel;
class QListView;

class TeamPicker : public QComboBox
{
    Q_OBJECT
    Q_PROPERTY( int teamid READ teamId WRITE setTeamId )

public:
    TeamPicker( QWidget * parent = 0, const char * name = 0 );
    int  teamId() const;
    void setTeamId( int id );

private:
    QMap< int, int > index2Id;
};

class TeamEditor : public TeamEditorBase
{
    Q_OBJECT

public:
    TeamEditor( QWidget * parent = 0, const char * name = 0 );
    void refreshTables();

protected slots:
    void updateTeamMembers( const QSqlRecord * record );
    void addPlayer();
    void removePlayer();
    void updateForm(){ emit formUpdated(); }

signals:
    void formUpdated();

private:
    Player2TeamView   player2teamView;
    Player2TeamCursor player2teamCursor;
    TeamCursor        teamCursor;
    PlayerCursor      playerCursor;
};

class Statistics : public StatisticsBase
{
    Q_OBJECT
public:
    Statistics( QWidget * parent = 0, const char * name = 0 );

public slots:
    void update();
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
