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

#include "qregexp.h"
#include "qvector.h"
#include "qnamespace.h"
#include "qatomic.h"
#include "qdebug.h"
#include "qstringlist.h"

#include "qsqlfield.h"

class QSqlRecordPrivate
{
public:
    QSqlRecordPrivate(): cnt(0)
    {
        ref = 1;
    }
    QSqlRecordPrivate(const QSqlRecordPrivate &other)
    {
        ref = 1;
        fi = other.fi;
        cnt = other.cnt;
    }
    ~QSqlRecordPrivate() {};
    void append(const QSqlField& field)
    {
        fi.append(field);
        cnt++;
    }
    void insert(int pos, const QSqlField& field)
    {
        if (pos < 0)
            return;
        if (pos == fi.size())
            append(field);
        if (pos > fi.size()) {
            fi.resize(pos + 1);
            cnt++;
        }
        fi[pos] = field;
    }
    void remove(int i)
    {
        if (i < 0 || i >= fi.count())
            return;
        if (fi.at(i).isValid())
            cnt--;
        fi[i] = QSqlField();
        // clean up some memory
        while (fi.count() && !fi.back().isValid())
            fi.pop_back();
    }
    void clear()
    {
        fi.clear();
        cnt = 0;
    }
    bool isEmpty()
    {
        return cnt == 0;
    }
    int count() const
    {
        return cnt;
    }
    bool contains(int i) const
    {
        return i >= 0 && i < (int)fi.count() && fi.at(i).isValid();
    }

    QString createField(int i, const QString& prefix) const
    {
        if (i < 0 || i >= (int)fi.count())
            return QString();
        QString f;
        if (!prefix.isEmpty())
            f = prefix + ".";
        f += fi.at(i).name();
        return f;
    }

public:
    QAtomic ref;

    QVector<QSqlField> fi;
    int cnt;
};

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
    Destroys the object and frees any allocated resources.
*/

QSqlRecord::~QSqlRecord()
{
    if (!--d->ref)
        delete d;
}

/*!
    Returns the value of the field located at position \a i in the
    record. If field \a i does not exist the resultant behavior is
    undefined.

    This function should be used with \l{QSqlQuery}s. When working
    with a QSqlCursor the \link QSqlCursor::value() value(const
    QString&)\endlink overload which uses field names is more
    appropriate.
*/

QCoreVariant QSqlRecord::value(int i) const
{
    return d->fi.value(i).value();
}

/*!
    \overload

    Returns the value of the field called \a name in the record. If
    field \a name does not exist the resultant behavior is undefined.
*/

QCoreVariant  QSqlRecord::value(const QString& name) const
{
    return value(position(name));
}

/*!
    Returns the name of the field at position \a i. If the field does
    not exist, an empty QString is returned.
*/

QString QSqlRecord::fieldName(int i) const
{
    return d->fi.value(i).name();
}

/*!
    Returns the position of the field called \a name within the
    record, or -1 if it cannot be found. Field names are not
    case-sensitive. If more than one field matches, the first one is
    returned.
*/

int QSqlRecord::position(const QString& name) const
{
    QString nm = name.toUpper();
    for (int i = 0; i < count(); ++i) {
        if (d->fi.at(i).name().toUpper() == nm) // TODO: case-insensitive comparison
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
    if (!d->contains(i)) {
        qWarning("QSqlRecord::field: index out of range: %d", i);
        return 0;
    }
    return &d->fi.at(i);
}

/*!
    \obsolete
    Use field() instead
*/

const QSqlField* QSqlRecord::fieldPtr(const QString& name) const
{
    int i = position(name);
    if (!d->contains(i)) {
        qWarning("QSqlRecord::field: index out of range: %d", i);
        return 0;
    }
    return &d->fi.at(i);
}

/*!
    returns the field at position \i
 */
QSqlField QSqlRecord::field(int i) const
{
    return d->fi.value(i);
}

/*! \overload
    returns the field named \a name
 */
QSqlField QSqlRecord::field(const QString &name) const
{
    return field(position(name));
}


/*!
    Append a copy of field \a field to the end of the record.
*/

void QSqlRecord::append(const QSqlField& field)
{
    detach();
    d->append(field);
}

/*!
    \fn QSqlRecord::insert(int pos, const QSqlField& field)

    \obsolete use replace() instead
*/

/*!
    Insert a copy of \a field at position \a pos. If a field already
    exists at \a pos, it is removed.
*/

void QSqlRecord::replace(int pos, const QSqlField& field)
{
    detach();
    d->insert(pos, field);
}

/*!
    Removes the field at \a pos. If \a pos does not exist, nothing
    happens.
*/

void QSqlRecord::remove(int pos)
{
    detach();
    d->remove(pos);
}

/*!
    Removes all the record's fields.

    \sa clearValues()
*/

void QSqlRecord::clear()
{
    detach();
    d->clear();
}

/*!
    Returns true if there are no fields in the record; otherwise
    returns false.
*/

bool QSqlRecord::isEmpty() const
{
    return d->isEmpty();
}


/*!
    Returns true if there is a field in the record called \a name;
    otherwise returns false.
*/

bool QSqlRecord::contains(const QString& name) const
{
    return position(name) >= 0;
}

/*!
    Clears the value of all fields in the record and sets each field
    to NULL.
*/

void QSqlRecord::clearValues()
{
    detach();
    int count = d->fi.count();
    for (int i = 0; i < count; ++i)
        d->fi[i].clear();
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
    setGenerated(position(name), generated);
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
    QSqlField fld(d->fi.at(i));
    QSqlField nf(fld.name(), fld.type(), fld.isRequired(), fld.length(), fld.precision(),
                 fld.defaultValue(), fld.typeID(), generated ? -1 : 1);
    d->fi[i] = nf;
}

/*!
    \overload

    Returns true if the field \a i is NULL or if there is no field at
    position \a i; otherwise returns false.

    \sa fieldName()
*/
bool QSqlRecord::isNull(int i) const
{
    return d->fi.value(i).isNull();
}

/*!
    Returns true if the field called \a name is NULL or if there is no
    field called \a name; otherwise returns false.

    \sa position()
*/
bool QSqlRecord::isNull(const QString& name) const
{
    return isNull(position(name));
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
    d->fi[i].clear();
}

/*!
    \overload

    Sets the value of the field called \a name to NULL. If the field
    does not exist, nothing happens.
*/
void QSqlRecord::setNull(const QString& name)
{
    setNull(position(name));
}


/*!
    Returns true if the record has a field called \a name and this
    field is to be generated (the default); otherwise returns false.

    \sa setGenerated()
*/
bool QSqlRecord::isGenerated(const QString& name) const
{
    return isGenerated(position(name));
}

/*!
    \overload

    Returns true if the record has a field at position \a i and this
    field is to be generated (the default); otherwise returns false.

    \sa setGenerated()
*/
bool QSqlRecord::isGenerated(int i) const
{
    return d->fi.value(i).isAutoGenerated() != 1;
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
    QString pflist;
    bool comma = false;
    for (int i = 0; i < count(); ++i){
        if (isGenerated(field(i).name())) {
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
    QStringList s;
    for (int i = 0; i < count(); ++i) {
        if (isGenerated(field(i).name()))
            s += d->createField(i, prefix);
    }
    return s;
}

/*!
    Returns the number of fields in the record.
*/

int QSqlRecord::count() const
{
    return d->count();
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
    d->fi[i].setValue(val);
}


/*!
    \overload

    Sets the value of the field called \a name to \a val. If the field
    does not exist, nothing happens.
*/

void QSqlRecord::setValue(const QString& name, const QCoreVariant& val)
{
    setValue(position(name), val);
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
        dbg.nospace() << "\n " << QString("%1: ").arg(i, 2) << r.field(i);
    return dbg.space();
}
#endif
#endif

#endif
