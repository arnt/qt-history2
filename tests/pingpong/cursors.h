#ifndef CURSORS_H
#define CURSORS_H

#include <qsqlcursor.h>

class QSqlRecord;

class MatchView : public QSqlCursor
{
public:
    MatchView();
};

class Player2TeamView : public QSqlCursor
{
public:
    Player2TeamView();
};


class MatchCursor : public QSqlCursor
{
public:
    MatchCursor();

protected:
    QVariant calculateField( const QString& name );
    void     primeInsert( QSqlRecord* buf );

    QSqlCursor * teamCr;
};

class Player2TeamCursor : public QSqlCursor
{
public:
    Player2TeamCursor();

protected:
    QVariant calculateField( const QString& name );
    void     primeInsert( QSqlRecord * buf );
};

class PlayerCursor : public QSqlCursor
{
public:
    PlayerCursor();

protected:
    void     primeInsert( QSqlRecord * buf );
};

class TeamCursor : public QSqlCursor
{
public:
    TeamCursor();

protected:
    void     primeInsert( QSqlRecord * buf );
};


#endif // CURSORS_H
