#ifndef INITDB_H
#define INITDB_H

#include <QtSql>

bool addBook(QSqlQuery &q, const QString &title, int year, const QVariant &authorId,
             const QVariant &genreId)
{
    q.addBindValue(title);
    q.addBindValue(year);
    q.addBindValue(authorId);
    q.addBindValue(genreId);
    return q.exec();
}

QSqlError initDb()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");

    if (!db.open())
        return db.lastError();

    QStringList tables = db.tables();
    if (tables.contains("books", Qt::CaseInsensitive)
        && tables.contains("authors", Qt::CaseInsensitive))
        return QSqlError();

    QSqlQuery q;
    if (!q.exec(QLatin1String("create table books(id integer primary key, title varchar, author integer, genre integer, year integer)")))
        return q.lastError();
    if (!q.exec(QLatin1String("create table authors(id integer primary key, name varchar, birthdate date)")))
        return q.lastError();
    if (!q.exec(QLatin1String("create table genres(id integer primary key, name varchar)")))
        return q.lastError();

    if (!q.prepare(QLatin1String("insert into authors(name, birthdate) values(?, ?)")))
        return q.lastError();
    q.addBindValue(QLatin1String("Isaac Asimov"));
    q.addBindValue(QDate(1920, 2, 1));
    if (!q.exec())
        return q.lastError();
    QVariant asimovId = q.lastInsertId();
    q.addBindValue(QLatin1String("Graham Greene"));
    q.addBindValue(QDate(1904, 10, 2));
    if (!q.exec())
        return q.lastError();
    QVariant greeneId = q.lastInsertId();

    if (!q.prepare(QLatin1String("insert into genres(name) values(?)")))
        return q.lastError();
    q.addBindValue(QLatin1String("Science Fiction"));
    if (!q.exec())
        return q.lastError();
    QVariant sfId = q.lastInsertId();
    q.addBindValue(QLatin1String("Fiction"));
    if (!q.exec())
        return q.lastError();
    QVariant fId = q.lastInsertId();

    if (!q.prepare(QLatin1String("insert into books(title, year, author, genre) values(?, ?, ?, ?)")))
        return q.lastError();
    if (!addBook(q, QLatin1String("Foundation"), 1951, asimovId, sfId))
        return q.lastError();
    if (!addBook(q, QLatin1String("Foundation and Empire"), 1952, asimovId, sfId))
        return q.lastError();
    if (!addBook(q, QLatin1String("Second Foundation"), 1953, asimovId, sfId))
        return q.lastError();
    if (!addBook(q, QLatin1String("Foundation's Edge"), 1982, asimovId, sfId))
        return q.lastError();
    if (!addBook(q, QLatin1String("Foundation and Earth"), 1986, asimovId, sfId))
        return q.lastError();
    if (!addBook(q, QLatin1String("Prelude to Foundation"), 1988, asimovId, sfId))
        return q.lastError();
    if (!addBook(q, QLatin1String("Forward the Foundation"), 1993, asimovId, sfId))
        return q.lastError();
    if (!addBook(q, QLatin1String("The Power and the Glory"), 1940, greeneId, fId))
        return q.lastError();
    if (!addBook(q, QLatin1String("The Third Man"), 1950, greeneId, fId))
        return q.lastError();
    if (!addBook(q, QLatin1String("Our Man in Havana"), 1958, greeneId, fId))
        return q.lastError();

    return QSqlError();
}

#endif

