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
#include "qsqlrecord_p.h"

#ifndef QT_NO_SQL

#include "qdebug.h"
#include "qstringlist.h"

QSqlRecordPrivate::QSqlRecordPrivate()
{
    ref = 1;
}

QSqlRecordPrivate::QSqlRecordPrivate(const QSqlRecordPrivate &other): fields(other.fields)
{
    ref = 1;
}

QSqlRecordPrivate::~QSqlRecordPrivate()
{
}

QSqlRecordPrivate *QSqlRecordPrivate::clone() const
{
    return new QSqlRecordPrivate(*this);
}

QString QSqlRecordPrivate::toString() const
{
    QString res;
    res.reserve(128);
    for (int i = 0; i < fields.count(); ++i)
        res.append(fields.at(i).name()).append(",");
    if (!res.isEmpty())
        res.truncate(res.size() - 1);
    return res;
}

/* Just for compat */
QString QSqlRecordPrivate::createField(int i, const QString &prefix) const
{
    QString f;
    if (!prefix.isEmpty())
        f = prefix + ".";
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
    the record in time O(1). If multiple QSqlRecord instances share
    the same data and one is modifying the record's data then this
    modifying instance makes a copy and modifies its private copy -
    thus it does not affect other instances.

    \sa QSqlRecordInfo
*/


/*!
    Constructs an empty record.
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
    Constructs a QSqlRecord with a custom private object \a p
 */
QSqlRecord::QSqlRecord(QSqlRecordPrivate &p): d(&p)
{
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
*/

QCoreVariant QSqlRecord::value(int i) const
{
    return d->fields.value(i).value();
}

/*!
    \overload

    Returns the value of the field called \a name in the record. If
    field \a name does not exist an invalid variant is returned.
*/

QCoreVariant QSqlRecord::value(const QString& name) const
{
    return value(indexOf(name));
}

/*!
    Returns the name of the field at position \a i. If the field does
    not exist, an empty QString is returned.
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

/*!
    returns the field at position \a i. If the position does not exist,
    an empty field is returned.
 */
QSqlField QSqlRecord::field(int i) const
{
    return d->fields.value(i);
}

/*! \overload
    returns the field named \a name
 */
QSqlField QSqlRecord::field(const QString &name) const
{
    return field(indexOf(name));
}


/*!
    Append a copy of field \a field to the end of the record.
*/

void QSqlRecord::append(const QSqlField& field)
{
    detach();
    d->fields.append(field);
}

/*!
    Inserts the field \a field at position \a pos.
 */
void QSqlRecord::insert(int pos, const QSqlField& field)
{
   detach();
   d->fields.insert(pos, field);
}

/*!
    Replaces the field at position \a pos with \a field.
    If \a pos does not exist, nothing happens.
*/

void QSqlRecord::replace(int pos, const QSqlField& field)
{
    if (!d->contains(pos))
        return;

    detach();
    d->fields[pos] = field;
}

/*!
    Removes the field at \a pos. If \a pos does not exist, nothing
    happens.
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

    \sa clearValues()
*/

void QSqlRecord::clear()
{
    detach();
    d->fields.clear();
}

/*!
    Returns true if there are no fields in the record; otherwise
    returns false.
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
    QSqlField fld(d->fields.at(i));
    QSqlField nf(fld.name(), fld.type(), fld.isRequired(), fld.length(), fld.precision(),
                 fld.defaultValue(), fld.typeID(), generated ? -1 : 1);
    d->fields[i] = nf;
}

/*!
    \overload

    Returns true if the field \a i is NULL or if there is no field at
    position \a i; otherwise returns false.

    \sa fieldName()
*/
bool QSqlRecord::isNull(int i) const
{
    return d->fields.value(i).isNull();
}

/*!
    Returns true if the field called \a name is NULL or if there is no
    field called \a name; otherwise returns false.

    \sa indexOf()
*/
bool QSqlRecord::isNull(const QString& name) const
{
    return isNull(indexOf(name));
}

/*!
    Sets the value of field \a i to NULL. If the field does not exist,
    nothing happens.
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


/*!
    Returns a list of all the record's field names as a string
    separated by \a sep.

    Note that fields which are not generated are \e not included (see
    \l{isGenerated()}). The returned string is suitable, for example, for
    generating SQL SELECT statements. If a \a prefix is specified,
    e.g. a table name, all fields are prefixed in the form:

    "\a{prefix}.\<fieldname\>"
*/

QString QSqlRecord::toString(const QString& prefix, const QString& sep) const
{
    // TODO - obsolete me
    QString pflist;
    bool comma = false;
    for (int i = 0; i < count(); ++i){
        if (d->fields.value(i).isAutoGenerated()) {
            if(comma)
                pflist += sep + " ";
            pflist += d->createField(i, prefix);
            comma = true;
        }
    }
    return pflist;
}

/*!
    Returns a list of all the record's field names, each having the
    prefix \a prefix.

    Note that fields which have generated set to false are \e not
    included. (See \l{isGenerated()}). If \a prefix is supplied, e.g.
    a table name, all fields are prefixed in the form:

    "\a{prefix}.\<fieldname\>"
*/

QStringList QSqlRecord::toStringList(const QString& prefix) const
{
    // TODO - obsolete me
    QStringList s;
    for (int i = 0; i < count(); ++i) {
        if (d->fields.value(i).isAutoGenerated())
            s += d->createField(i, prefix);
    }
    return s;
}

/*!
    Returns the number of fields in the record.
*/

int QSqlRecord::count() const
{
    return d->fields.count();
}

/*!
    Sets the value of the field at position \a i to \a val. If the
    field does not exist, nothing happens.
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
    if (d->ref == 1)
        return;
    QSqlRecordPrivate *x = d->clone();
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
        delete x;
}

#if !defined(Q_OS_MAC) || QT_MACOSX_VERSION >= 0x1030
#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const QSqlRecord &r)
{
    dbg.nospace() << "QSqlRecord(" << r.count() << ")";
    for (int i = 0; i < r.count(); ++i)
        dbg.nospace() << "\n " << QString("%1: ").arg(i, 2) << r.field(i);
    return dbg.space();
}
#endif
#endif

#endif
