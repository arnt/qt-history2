#ifndef DIALOGS_H
#define DIALOGS_H

#include <qdialog.h>
#include <qsqltable.h>
#include <qframe.h>
#include <qcombobox.h>
#include <qspinbox.h>
#include <qlineedit.h>

#include "pingpongapp.h"
#include "cursors.h"


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

class MatchDialog : public QDialog
{
    Q_OBJECT

public:
    typedef enum Mode { Insert, Update, Delete };    
    
    MatchDialog( QSqlRecord* buf, Mode mode, QWidget * parent = 0,
		 const char * name = 0 );
public slots:
    void close();
    void execute();
    
private slots:
    void updateSets();

private:
    QSqlForm * form;
    QSqlRecord* matchRecord;
    TeamPicker * wteam;
    TeamPicker * lteam;
    QSpinBox * wins;
    QSpinBox * losses;
    QLineEdit * sets;
    Mode mMode;        
};

class EditTeamsDialog : public QDialog
{
    Q_OBJECT

public:
    EditTeamsDialog( QWidget * parent = 0, const char * name = 0 );
    
protected slots:
    void updateTeamMembers( const QSqlRecord * record );
    void addPlayer();
    void removePlayer();
    
private:
    QSqlTable * teamTable;
    QSqlTable * playerTable;
    QSqlTable * player2teamTable;
    
    QLabel * player2teamLabel;
    Player2TeamCursor player2teamCursor;
    TeamCursor   teamCursor;
    PlayerCursor playerCursor;
};

#endif // DIALOGS_H

