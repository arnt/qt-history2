/****************************************************************************
**
** Definition of QSqlSelectCursor class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
** EDITIONS: FREE, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSQLSELECTCURSOR_H
#define QSQLSELECTCURSOR_H

#ifndef QT_H
#include "qsqlcursor.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlSelectCursorPrivate;

class Q_COMPAT_EXPORT QSqlSelectCursor : public QSqlCursor
{
public:
    QSqlSelectCursor(const QString& query = QString(), QSqlDatabase db = QSqlDatabase());
    QSqlSelectCursor(const QSqlSelectCursor& other);
    ~QSqlSelectCursor();
    bool exec(const QString& query);
    bool select() { return QSqlCursor::select(); }

protected:
    QSqlIndex primaryIndex(bool = true) const { return QSqlIndex(); }
    QSqlIndex index(const QStringList&) const { return QSqlIndex(); }
    QSqlIndex index(const QString&) const { return QSqlIndex(); }
    QSqlIndex index(const char*) const { return QSqlIndex(); }
    void setPrimaryIndex(const QSqlIndex&) {}
    void append(const QSqlFieldInfo&) {}
    void insert(int, const QSqlFieldInfo&) {}
    void remove(int) {}
    void clear() {}
    void setGenerated(const QString&, bool) {}
    void setGenerated(int, bool) {}
    QSqlRecord*        editBuffer(bool = false) { return 0; }
    QSqlRecord*        primeInsert() { return 0; }
    QSqlRecord*        primeUpdate() { return 0; }
    QSqlRecord*        primeDelete() { return 0; }
    int        insert(bool = true) { return 0; }
    int        update(bool = true) { return 0; }
    int        del(bool = true) { return 0; }
    void setMode(int) {}

    void setSort(const QSqlIndex&) {}
    QSqlIndex sort() const { return QSqlIndex(); }
    void setFilter(const QString&) {}
    QString filter() const { return QString(); }
    void setName(const QString&, bool = true) {}
    QString name() const { return QString(); }
    QString toString(const QString& = QString(), const QString& = ",") const { return QString(); }
    bool select(const QString &, const QSqlIndex& = QSqlIndex());

private:
    void populateCursor();

    QSqlSelectCursorPrivate * d;
};

#endif // QT_NO_SQL
#endif // QSQLSELECTCURSOR_H
