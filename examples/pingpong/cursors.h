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

protected:
    void     primeInsert( QSqlRecord * buf );
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

protected:
    void     primeInsert( QSqlRecord* buf );
};

//
// Player2TeamCursor
//
class Player2TeamCursor : public QSqlCursor
{
public:
    Player2TeamCursor();

protected:
    QVariant calculateField( const QString& name );
    void     primeInsert( QSqlRecord * buf );
};

//
// PlayerCursor
//
class PlayerCursor : public QSqlCursor
{
public:
    PlayerCursor();

protected:
    void     primeInsert( QSqlRecord * buf );
};

#endif // CURSORS_H
