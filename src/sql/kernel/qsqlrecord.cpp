/****************************************************************************
**
** Implementation of QSqlRecord class.
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

#include "qsqlrecord.h"

#ifndef QT_NO_SQL

#include "qdebug.h"
#include "qstringlist.h"
#include "qatomic.h"
#include "qsqlfield.h"
#include "qstring.h"
#include "qvector.h"

class QSqlRecordPrivate
{
public:
    QSqlRecordPrivate();
    QSqlRecordPrivate(const QSqlRecordPrivate &other);

    inline bool contains(int i) { return i >= 0 && i < fields.count(); }
    QString createField(int i, const QString &prefix) const;

    QVector<QSqlField> fields;
    QAtomic ref;
};

QSqlRecordPrivate::QSqlRecordPrivate()
{
    ref = 1;
}

QSqlRecordPrivate::QSqlRecordPrivate(const QSqlRecordPrivate &other): fields(other.fields)
{
    ref = 1;
}

/*! \internal
    Just for compat
*/
QString QSqlRecordPrivate::createField(int i, const QString &prefix) const
{
    QString f;
    if (!prefix.isEmpty())
        f = prefix + QLatin1Char('.');
    f += fields.at(i).name();
    return f;
}

/*!
    \class QSqlRecord qsqlfield.h
    \brief The QSqlRecord class encapsulates a database record, i.e. a
    set of database fields.

    \ingroup database
    \module sql

    The QSqlRecord class encapsulates the functionality and
    characteristics of a database record (usually a table or view within
    the database). QSqlRecords support adding and removing fields as
    well as setting and retrieving field values.

    QSqlRecord is implicitly shared. This means you can make copies of
    a record in O(1) time. If multiple QSqlRecord instances share the
    same data and one is modifying the record's data then this
    modifying instance makes a copy and modifies its private copy -
    thus it does not affect other instances.

    A record's field's can be set by name or position with setValue();
    if you want to set a field to NULL use setNull(). To find the
    position of a field by name use indexOf(), and to find the name of
    a field at a particular position use fieldName(). Use field() to
    retrieve a QSqlField object for a given field. Use contains() to
    see if the record contains a particular field name.

    When queries are generated to be executed on the database only
    those fields for which isGenerated() is true are included in the
    generated SQL.

    A record can have fields added with append() or insert(), replaced
    with replace(), and removed with remove(). All the fields can be
    removed with clear(). The number of fields is given by count();
    all their values can be cleared (to NULL) using clearValues(). The
    names of all the fields is returned by toString() and by
    toStringList().
*/


/*!
    Constructs an empty record. isEmpty() will return true and count()
    will return 0.

    \sa append() insert()
*/

QSqlRecord::QSqlRecord()
{
    d = new QSqlRecordPrivate();
}

/*!
    Constructs a copy of \a other.
*/

QSqlRecord::QSqlRecord(const QSqlRecord& other)
{
    d = other.d;
    ++d->ref;
}

/*!
    Sets the record equal to \a other.
*/

QSqlRecord& QSqlRecord::operator=(const QSqlRecord& other)
{
    qAtomicAssign(d, other.d);
    return *this;
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSqlRecord::~QSqlRecord()
{
    if (!--d->ref)
        delete d;
}

/*!
    Returns the value of the field located at position \a i in the
    record. If field \a i does not exist an invalid variant is returned.

    This function should be used with \l{QSqlQuery}s. When working
    with a QSqlCursor the \link QSqlCursor::value() value(const
    QString&)\endlink overload which uses field names is more
    appropriate.

    \sa fieldName() isNull()
*/

QCoreVariant QSqlRecord::value(int i) const
{
    return d->fields.value(i).value();
}

/*!
    \overload

    Returns the value of the field called \a name in the record. If
    field \a name does not exist an invalid variant is returned.

    \sa indexOf()
*/

QCoreVariant QSqlRecord::value(const QString& name) const
{
    return value(indexOf(name));
}

/*!
    Returns the name of the field at position \a i. If the field does
    not exist, an empty string is returned.

    \sa indexOf()
*/

QString QSqlRecord::fieldName(int i) const
{
    return d->fields.value(i).name();
}

/*!
    Returns the position of the field called \a name within the
    record, or -1 if it cannot be found. Field names are not
    case-sensitive. If more than one field matches, the first one is
    returned.

    \sa fieldName()
*/

int QSqlRecord::indexOf(const QString& name) const
{
    QString nm = name.toUpper();
    for (int i = 0; i < count(); ++i) {
        if (d->fields.at(i).name().toUpper() == nm) // TODO: case-insensitive comparison
            return i;
    }
    return -1;
}

#ifdef QT_COMPAT
/*!
    \obsolete
    Use field() instead
*/
const QSqlField* QSqlRecord::fieldPtr(int i) const
{
    if (!d->contains(i))
        return 0;

    return &d->fields.at(i);
}

/*!
    \obsolete
    Use field() instead
*/

const QSqlField* QSqlRecord::fieldPtr(const QString& name) const
{
    int i = indexOf(name);
    if (!d->contains(i))
        return 0;

    return &d->fields.at(i);
}
#endif //QT_COMPAT

/*!
    Returns the field at position \a i. If the position is out of
    range, an empty field is returned.
 */
QSqlField QSqlRecord::field(int i) const
{
    return d->fields.value(i);
}

/*! \overload
    Returns the field called \a name.
 */
QSqlField QSqlRecord::field(const QString &name) const
{
    return field(indexOf(name));
}


/*!
    Append a copy of field \a field to the end of the record.

    \sa insert() replace() remove()
*/

void QSqlRecord::append(const QSqlField& field)
{
    detach();
    d->fields.append(field);
}

/*!
    Inserts the field \a field at position \a pos in the record.

    \sa append() replace() remove()
 */
void QSqlRecord::insert(int pos, const QSqlField& field)
{
   detach();
   d->fields.insert(pos, field);
}

/*!
    Replaces the field at position \a pos with the given \a field. If
    \a pos is out of range, nothing happens.

    \sa append() insert() remove()
*/

void QSqlRecord::replace(int pos, const QSqlField& field)
{
    if (!d->contains(pos))
        return;

    detach();
    d->fields[pos] = field;
}

/*!
    Removes the field at position \a pos. If \a pos is out of range,
    nothing happens.

    \sa append() insert() replace()
*/

void QSqlRecord::remove(int pos)
{
    if (!d->contains(pos))
        return;

    detach();
    d->fields.remove(pos);
}

/*!
    Removes all the record's fields.

    \sa clearValues() isEmpty()
*/

void QSqlRecord::clear()
{
    detach();
    d->fields.clear();
}

/*!
    Returns true if there are no fields in the record; otherwise
    returns false.

    \sa append() insert() clear()
*/

bool QSqlRecord::isEmpty() const
{
    return d->fields.isEmpty();
}


/*!
    Returns true if there is a field in the record called \a name;
    otherwise returns false.
*/

bool QSqlRecord::contains(const QString& name) const
{
    return indexOf(name) >= 0;
}

/*!
    Clears the value of all fields in the record and sets each field
    to NULL.

    \sa setValue()
*/

void QSqlRecord::clearValues()
{
    detach();
    int count = d->fields.count();
    for (int i = 0; i < count; ++i)
        d->fields[i].clear();
}

/*!
    Sets the generated flag for the field called \a name to \a
    generated. If the field does not exist, nothing happens. Only
    fields that have \a generated set to true are included in the SQL
    that is generated, e.g. by QSqlCursor.

    \sa isGenerated()
*/

void QSqlRecord::setGenerated(const QString& name, bool generated)
{
    setGenerated(indexOf(name), generated);
}

/*!
    \overload

    Sets the generated flag for the field \a i to \a generated.

    \sa isGenerated()
*/

void QSqlRecord::setGenerated(int i, bool generated)
{
    if (!d->contains(i))
        return;
    detach();
    d->fields[i].setAutoGenerated(generated ? QSqlField::No : QSqlField::Yes);
}

/*!
    \overload

    Returns true if the field \a i is NULL or if there is no field at
    position \a i; otherwise returns false.
*/
bool QSqlRecord::isNull(int i) const
{
    return d->fields.value(i).isNull();
}

/*!
    Returns true if the field called \a name is NULL or if there is no
    field called \a name; otherwise returns false.

    \sa setNull()
*/
bool QSqlRecord::isNull(const QString& name) const
{
    return isNull(indexOf(name));
}

/*!
    Sets the value of field \a i to NULL. If the field does not exist,
    nothing happens.

    \sa setValue()
*/
void QSqlRecord::setNull(int i)
{
    if (!d->contains(i))
        return;
    detach();
    d->fields[i].clear();
}

/*!
    \overload

    Sets the value of the field called \a name to NULL. If the field
    does not exist, nothing happens.
*/
void QSqlRecord::setNull(const QString& name)
{
    setNull(indexOf(name));
}


/*!
    Returns true if the record has a field called \a name and this
    field is to be generated (the default); otherwise returns false.

    \sa setGenerated()
*/
bool QSqlRecord::isGenerated(const QString& name) const
{
    return isGenerated(indexOf(name));
}

/*! \obsolete
    \overload

    Returns true if the record has a field at position \a i and this
    field is to be generated (the default); otherwise returns false.

    \sa setGenerated()
*/
bool QSqlRecord::isGenerated(int i) const
{
    return d->fields.value(i).isAutoGenerated() != 1;
}

#ifdef QT_COMPAT
/*!
    Returns a list of all the record's field names as a string
    separated by \a sep.

    Note that fields for which isGenerated() returns false are \e not
    included. The returned string is suitable, for example, for
    generating SQL \c SELECT statements. If a \a prefix is specified,
    e.g. a table name, all fields are prefixed in the form:

    "\a{prefix}.\<fieldname\>"
*/

QString QSqlRecord::toString(const QString& prefix, const QString& sep) const
{
    // TODO - obsolete me
    QString pflist;
    bool comma = false;
    for (int i = 0; i < count(); ++i) {
        if (d->fields.value(i).isAutoGenerated() != QSqlField::Yes) {
            if(comma)
                pflist += sep + QLatin1Char(' ');
            pflist += d->createField(i, prefix);
            comma = true;
        }
    }
    return pflist;
}

/*!
    Returns a list of all the record's field names, each having the
    prefix \a prefix.

    Note that fields for which isGenerated() returns false are \e not
    included. If \a prefix is supplied, e.g. a table name, all fields
    are prefixed in the form:

    "\a{prefix}.\<fieldname\>"
*/

QStringList QSqlRecord::toStringList(const QString& prefix) const
{
    // TODO - obsolete me
    QStringList s;
    for (int i = 0; i < count(); ++i) {
        if (d->fields.value(i).isAutoGenerated() != QSqlField::Yes)
            s += d->createField(i, prefix);
    }
    return s;
}
#endif //QT_COMPAT

/*!
    Returns the number of fields in the record.

    \sa isEmpty()
*/

int QSqlRecord::count() const
{
    return d->fields.count();
}

/*!
    Sets the value of the field at position \a i to \a val. If the
    field does not exist, nothing happens.

    \sa setNull()
*/

void QSqlRecord::setValue(int i, const QCoreVariant& val)
{
    if (!d->contains(i))
        return;
    detach();
    d->fields[i].setValue(val);
}


/*!
    \overload

    Sets the value of the field called \a name to \a val. If the field
    does not exist, nothing happens.
*/

void QSqlRecord::setValue(const QString& name, const QCoreVariant& val)
{
    setValue(indexOf(name), val);
}


/*! \internal
*/
void QSqlRecord::detach()
{
    qAtomicDetach(d);
}

#if !defined(Q_OS_MAC) || QT_MACOSX_VERSION >= 0x1030
#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const QSqlRecord &r)
{
    dbg.nospace() << "QSqlRecord(" << r.count() << ")";
    for (int i = 0; i < r.count(); ++i)
        dbg.nospace() << QLatin1String("\n ") << QString::fromLatin1("%1: ").arg(i, 2)
                      << r.field(i);
    return dbg.space();
}
#endif
#endif

#endif
