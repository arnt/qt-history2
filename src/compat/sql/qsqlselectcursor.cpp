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

#include "qsqlselectcursor.h"
#include "qsqldriver.h"
#include "qsqlrecordinfo.h"

#ifndef QT_NO_SQL

class QSqlSelectCursorPrivate
{
public:
    QSqlSelectCursorPrivate() : populated(false) {}
    QString query;
    bool populated : 1;
};

/*!
    \class QSqlSelectCursor qsqlselectcursor.h
    \brief The QSqlSelectCursor class provides browsing of general SQL
    SELECT statements.

    \ingroup database
    \module sql

    QSqlSelectCursor is a convenience class that makes it possible to
    display result sets from general SQL \c SELECT statements in
    data-aware Qt widgets. QSqlSelectCursor is read-only and does not
    support \c INSERT, \c UPDATE or \c DELETE operations.

    Pass the query in at construction time, or use the
    QSqlSelectCursor::exec() function.

    Example:
    \code
    ...
    QSqlSelectCursor* cur = new QSqlSelectCursor("SELECT id, firstname, lastname FROM author");
    QDataTable* table = new QDataTable(this);
    table->setSqlCursor(cur, true, true);
    table->refresh();
    ...
    cur->exec("SELECT * FROM books");
    table->refresh();
    ...
    \endcode
*/

/*!
    Constructs a read only cursor on database \a db using the query \a query.
 */
QSqlSelectCursor::QSqlSelectCursor(const QString& query, QSqlDatabase db)
    : QSqlCursor(QString(), false, db)
{
    d = new QSqlSelectCursorPrivate;
    d->query = query;
    QSqlCursor::setMode(ReadOnly);
    if (!query.isEmpty())
        exec(query);
}

/*! Constructs a copy of \a other */
QSqlSelectCursor::QSqlSelectCursor(const QSqlSelectCursor& other)
    : QSqlCursor(other)
{
    d = new QSqlSelectCursorPrivate;
    d->query = other.d->query;
    d->populated = other.d->populated;
}

/*! Destroys the object and frees any allocated resources */
QSqlSelectCursor::~QSqlSelectCursor()
{
    delete d;
}

/*! \internal */
bool QSqlSelectCursor::exec(const QString& query)
{
    d->query = query;
    bool ret = QSqlCursor::exec(query);
    if (ret) {
        QSqlCursor::clear();
        populateCursor();
    }
    return ret;
}

/*! \fn bool QSqlSelectCursor::select()
    \internal
*/

/*! \internal */
bool QSqlSelectCursor::select(const QString&, const QSqlIndex&)
{
    bool ret = QSqlCursor::exec(d->query);
    if (ret && !d->populated)
        populateCursor();
    return ret;
}

/*! \internal */
void QSqlSelectCursor::populateCursor()
{
    QSqlRecordInfo inf = QSqlRecordInfo(record());
    for (QSqlRecordInfo::const_iterator it = inf.begin(); it != inf.end(); ++it)
        QSqlCursor::append(*it);
    d->populated = true;
}

/*! \fn QSqlIndex QSqlSelectCursor::primaryIndex(bool) const
    \internal
*/

/*! \fn QSqlIndex QSqlSelectCursor::index(const QStringList&) const
    \internal
*/

/*! \fn QSqlIndex QSqlSelectCursor::index(const QString&) const
    \internal
*/

/*! \fn QSqlIndex QSqlSelectCursor::index(const char*) const
    \internal
*/

/*! \fn void QSqlSelectCursor::setPrimaryIndex(const QSqlIndex&)
    \internal
*/

/*! \fn void QSqlSelectCursor::append(const QSqlFieldInfo&)
    \internal
*/

/*! \fn void QSqlSelectCursor::insert(int, const QSqlFieldInfo&)
    \internal
*/

/*! \fn void QSqlSelectCursor::remove(int)
    \internal
*/

/*! \fn void QSqlSelectCursor::clear()
    \internal
*/

/*! \fn void QSqlSelectCursor::setGenerated(const QString&, bool)
    \internal
*/

/*! \fn void QSqlSelectCursor::setGenerated(int, bool)
    \internal
*/

/*! \fn QSqlRecord* QSqlSelectCursor::editBuffer(bool)
    \internal
*/

/*! \fn QSqlRecord* QSqlSelectCursor::primeInsert()
    \internal
*/

/*! \fn QSqlRecord* QSqlSelectCursor::primeUpdate()
    \internal
*/

/*! \fn QSqlRecord* QSqlSelectCursor::primeDelete()
    \internal
*/

/*! \fn int QSqlSelectCursor::insert(bool)
    \internal
*/

/*! \fn int QSqlSelectCursor::update(bool)
    \internal
*/

/*! \fn int QSqlSelectCursor::del(bool)
    \internal
*/

/*! \fn void QSqlSelectCursor::setMode(int)
    \internal
*/

/*! \fn void QSqlSelectCursor::setSort(const QSqlIndex&)
    \internal
*/

/*! \fn QSqlIndex QSqlSelectCursor::sort() const
    \internal
*/

/*! \fn void QSqlSelectCursor::setFilter(const QString&)
    \internal
*/

/*! \fn QString QSqlSelectCursor::filter() const
    \internal
*/

/*! \fn void QSqlSelectCursor::setName(const QString&, bool)
    \internal
*/

/*! \fn QString QSqlSelectCursor::name() const
    \internal
*/

/*! \fn QString QSqlSelectCursor::toString(const QString&, const QString&) const
    \internal
*/
#endif // QT_NO_SQL
