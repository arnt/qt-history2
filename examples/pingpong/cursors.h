#ifndef CURSORS_H
#define CURSORS_H

#include <qsqlcursor.h>

class QSqlRecord;

//
// TeamCursor
//
class TeamCursor : public QSqlCursor
{
public:
    TeamCursor();
    QSqlRecord* primeInsert();
};

//
// MatchView
//
class MatchView : public QSqlCursor
{
public:
    MatchView();
};

//
// Player2TeamView
//
class Player2TeamView : public QSqlCursor
{
public:
    Player2TeamView();
};

//
// MatchCursor
//
class MatchCursor : public QSqlCursor
{
public:
    MatchCursor();
    QSqlRecord* primeInsert();
};

//
// Player2TeamCursor
//
class Player2TeamCursor : public QSqlCursor
{
public:
    Player2TeamCursor();
    QSqlRecord* primeInsert();
protected:
    QVariant calculateField( const QString& name );
};

//
// PlayerCursor
//
class PlayerCursor : public QSqlCursor
{
public:
    PlayerCursor();
    QSqlRecord* primeInsert();
};

#endif // CURSORS_H
