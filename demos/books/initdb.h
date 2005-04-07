#ifndef INITDB_H
#define INITDB_H

#include <QtSql>

void addBook(QSqlQuery &q, const QString &title, int year, const QVariant &authorId,
             const QVariant &genreId)
{
    q.addBindValue(title);
    q.addBindValue(year);
    q.addBindValue(authorId);
    q.addBindValue(genreId);
    q.exec();
}

QVariant addGenre(QSqlQuery &q, const QString &name)
{
    q.addBindValue(name);
    q.exec();
    return q.lastInsertId();
}

QVariant addAuthor(QSqlQuery &q, const QString &name, const QDate &birthdate)
{
    q.addBindValue(name);
    q.addBindValue(birthdate);
    q.exec();
    return q.lastInsertId();
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
    QVariant asimovId = addAuthor(q, QLatin1String("Isaac Asimov"), QDate(1920, 2, 1));
    QVariant greeneId = addAuthor(q, QLatin1String("Graham Greene"), QDate(1904, 10, 2));
    QVariant pratchettId = addAuthor(q, QLatin1String("Terry Pratchett"), QDate(1948, 4, 28));

    if (!q.prepare(QLatin1String("insert into genres(name) values(?)")))
        return q.lastError();
    QVariant sfiction = addGenre(q, QLatin1String("Science Fiction"));
    QVariant fiction = addGenre(q, QLatin1String("Fiction"));
    QVariant fantasy = addGenre(q, QLatin1String("Fantasy"));

    if (!q.prepare(QLatin1String("insert into books(title, year, author, genre) values(?, ?, ?, ?)")))
        return q.lastError();
    addBook(q, QLatin1String("Foundation"), 1951, asimovId, sfiction);
    addBook(q, QLatin1String("Foundation and Empire"), 1952, asimovId, sfiction);
    addBook(q, QLatin1String("Second Foundation"), 1953, asimovId, sfiction);
    addBook(q, QLatin1String("Foundation's Edge"), 1982, asimovId, sfiction);
    addBook(q, QLatin1String("Foundation and Earth"), 1986, asimovId, sfiction);
    addBook(q, QLatin1String("Prelude to Foundation"), 1988, asimovId, sfiction);
    addBook(q, QLatin1String("Forward the Foundation"), 1993, asimovId, sfiction);
    addBook(q, QLatin1String("The Power and the Glory"), 1940, greeneId, fiction);
    addBook(q, QLatin1String("The Third Man"), 1950, greeneId, fiction);
    addBook(q, QLatin1String("Our Man in Havana"), 1958, greeneId, fiction);
    addBook(q, QLatin1String("Guards! Guards!"), 1989, pratchettId, fantasy);
    addBook(q, QLatin1String("Night Watch"), 2002, pratchettId, fantasy);
    addBook(q, QLatin1String("Going Postal"), 2004, pratchettId, fantasy);

    return QSqlError();
}

#endif

