/****************************************************************************
**
** Implementation of QSqlIndex class.
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

#include "qsqlindex.h"

#ifndef QT_NO_SQL

/*!
    \class QSqlIndex qsqlindex.h
    \brief The QSqlIndex class provides functions to manipulate and
    describe QSqlCursor and QSqlDatabase indexes.

    \ingroup database
    \module sql

    This class is used to describe and manipulate QSqlCursor and
    QSqlDatabase indexes. An index refers to a single table or view
    in a database. Information about the fields that comprise the
    index can be used to generate SQL statements, or to affect the
    behavior of a \l QSqlCursor object.

    Normally, QSqlIndex objects are created by \l QSqlDatabase or
    QSqlCursor.
*/

/*!
    Constructs an empty index using the cursor name \a cursorname and
    index name \a name.
*/

QSqlIndex::QSqlIndex(const QString& cursorname, const QString& name)
    : QSqlRecord(), cursor(cursorname), nm(name)
{

}

/*!
    Constructs a copy of \a other.
*/

QSqlIndex::QSqlIndex(const QSqlIndex& other)
    : QSqlRecord(other), cursor(other.cursor), nm(other.nm), sorts(other.sorts)
{
}

/*!
    Sets the index equal to \a other.
*/

QSqlIndex& QSqlIndex::operator=(const QSqlIndex& other)
{
    cursor = other.cursor;
    nm = other.nm;
    sorts = other.sorts;
    QSqlRecord::operator=(other);
    return *this;
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSqlIndex::~QSqlIndex()
{

}

/*!
    Sets the name of the index to \a name.
*/

void QSqlIndex::setName(const QString& name)
{
    nm = name;
}

/*!
    \fn QString QSqlIndex::name() const

    Returns the name of the index.
*/

/*!
    Appends the field \a field to the list of indexed fields. The
    field is appended with an ascending sort order.
*/

void QSqlIndex::append(const QSqlField& field)
{
    append(field, false);
}

/*!
    \overload

    Appends the field \a field to the list of indexed fields. The
    field is appended with an ascending sort order, unless \a desc is
    true.
*/

void QSqlIndex::append(const QSqlField& field, bool desc)
{
    sorts.append(desc);
    QSqlRecord::append(field);
}


/*!
    Returns true if field \a i in the index is sorted in descending
    order; otherwise returns false.
*/

bool QSqlIndex::isDescending(int i) const
{
    if (i >= 0 && i < sorts.size())
        return sorts[i];
    return false;
}

/*!
    If \a desc is true, field \a i is sorted in descending order.
    Otherwise, field \a i is sorted in ascending order (the default).
    If the field does not exist, nothing happens.
*/

void QSqlIndex::setDescending(int i, bool desc)
{
    if (i >= 0 && i < sorts.size())
        sorts[i] = desc;
}

/*!
    \reimp

    Returns a comma-separated list of all the index's field names as a
    string. This string is suitable, for example, for generating a
    SQL SELECT statement. Only generated fields are included in the
    list (see \l{isGenerated()}). If a \a prefix is specified, e.g. a
    table name, it is prepended before all field names in the form:

    "\a{prefix}.<fieldname>"

    If \a sep is specified, each field is separated by \a sep. If \a
    verbose is true (the default), each field contains a suffix
    indicating an ASCending or DESCending sort order.
*/

QString QSqlIndex::toString(const QString& prefix, const QString& sep, bool verbose) const
{
    QString s;
    bool comma = false;
    for (int i = 0; i < count(); ++i) {
        if(comma)
            s += sep + " ";
        s += createField(i, prefix, verbose);
        comma = true;
    }
    return s;
}

/*!
    \reimp

    Returns a list of all the index's field names. Only generated
    fields are included in the list (see \l{isGenerated()}). If a \a
    prefix is specified, e.g. a table name, all fields are prefixed in
    the form:

    "\a{prefix}.<fieldname>"

    If \a verbose is true (the default), each field contains a suffix
    indicating an ASCending or DESCending sort order.

    Note that if you want to iterate over the list, you should iterate
    over a copy, e.g.
    \code
    QStringList list = myIndex.toStringList();
    QStringList::Iterator it = list.begin();
    while(it != list.end()) {
        myProcessing(*it);
        ++it;
    }
    \endcode

*/
QStringList QSqlIndex::toStringList(const QString& prefix, bool verbose) const
{
    QStringList s;
    for (int i = 0; i < count(); ++i)
        s += createField(i, prefix, verbose);
    return s;
}

/*! \internal

  Creates a string representing the field number \a i using prefix \a
  prefix. If \a verbose is true, ASC or DESC is included in the field
  description if the field is sorted in ASCending or DESCending order.
*/

QString QSqlIndex::createField(int i, const QString& prefix, bool verbose) const
{
    QString f;
    if (!prefix.isEmpty())
        f += prefix + ".";
    f += field(i)->name();
    if (verbose)
        f += " " + QString((isDescending(i) ? "DESC" : "ASC"));
    return f;
}

/*!
    \fn QString QSqlIndex::cursorName() const

    Returns the name of the cursor which the index is associated with.
*/


/*!
    Sets the name of the cursor that the index is associated with to
    \a cursorName.
*/
void QSqlIndex::setCursorName(const QString& cursorName)
{
    cursor = cursorName;
}

#endif
