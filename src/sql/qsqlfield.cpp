/****************************************************************************
**
** Implementation of QSqlField class.
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

#ifndef QT_NO_SQL

#include "qsqlfield.h"
#include "qatomic.h"
#include "qdebug.h"

class QSqlFieldPrivate
{
public:
    QSqlFieldPrivate(const QString &name,
              QCoreVariant::Type type,
              int required, int length, int prec,
              const QCoreVariant& defValue,
              int sqlType, int autogen) :
        nm(name), ro(false), type(type), req(required),
        len(length), pr(prec), def(defValue), tp(sqlType), gen(autogen)
    {
        ref = 1;
    }

    QSqlFieldPrivate& operator=(const QSqlFieldPrivate& other)
    {
        nm = other.nm;
        ro = other.ro;
        type = other.type;
        req = other.req;
        len = other.len;
        pr = other.pr;
        def = other.def;
        tp = other.tp;
        gen = other.gen;
        return *this;
    }

    bool operator==(const QSqlFieldPrivate& other) const
    {
        return (nm == other.nm
                && ro == other.ro
                && type == other.type
                && req == other.req
                && len == other.len
                && pr == other.pr
                && def == other.def
                && tp == other.tp
                && gen == other.gen);
    }

    QAtomic ref;
    QString nm;
    uint ro: 1;
    QCoreVariant::Type type;
    int req;
    int len;
    int pr;
    QCoreVariant def;
    int tp;
    int gen;
};


/*!
    \class QSqlField qsqlfield.h
    \brief The QSqlField class manipulates the fields in SQL database tables
    and views.

    \ingroup database
    \module sql

    QSqlField represents the characteristics of a single column in a
    database table or view, such as the data type and column name. A
    field also contains the value of the database column, which can be
    viewed or changed.

    Field data values are stored as QCoreVariants. Using an incompatible
    type is not permitted. For example:

    \code
    QSqlField f("myfield", QCoreVariant::Int);
    f.setValue(QPixmap());  // will not work
    \endcode

    However, the field will attempt to cast certain data types to the
    field data type where possible:

    \code
    QSqlField f("myfield", QCoreVariant::Int);
    f.setValue(QString("123")); // casts QString to int
    \endcode

    QSqlField objects are rarely created explicitly in application
    code. They are usually accessed indirectly through \l QSqlRecord
    or \l QSqlCursor which already contain a list of fields. For
    example:

    \code
    QSqlCursor cur("Employee");        // create cursor using the 'Employee' table
    QSqlField* f = cur.field("name");  // use the 'name' field
    f->setValue("Dave");               // set field value
    ...
    \endcode

    In practice we rarely need to extract a pointer to a field at all.
    The previous example would normally be written:

    \code
    QSqlCursor cur("Employee");
    cur.setValue("name", "Dave");
    ...
    \endcode
*/

/*!
    Constructs an empty field called \a fieldName of type \a type.
*/
QSqlField::QSqlField(const QString& fieldName,
              QCoreVariant::Type type,
              int required,
              int fieldLength,
              int prec,
              const QCoreVariant& defValue,
              int sqlType,
              int autogen)
{
    d = new QSqlFieldPrivate(fieldName, type, required,
                             fieldLength, prec, defValue, sqlType, autogen);
    val.cast(type);
}

/*!
    Constructs a copy of \a other.
*/

QSqlField::QSqlField(const QSqlField& other)
{
    d = other.d;
    ++d->ref;
    val = other.val;
}

/*!
    Sets the field equal to \a other.
*/

QSqlField& QSqlField::operator=(const QSqlField& other)
{
    QSqlFieldPrivate *x = other.d;
    ++x->ref;
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
        delete x;
    val = other.val;
    return *this;
}

/*!
    Returns true if the field is equal to \a other; otherwise returns
    false.
*/
bool QSqlField::operator==(const QSqlField& other) const
{
    return ((d == other.d || *d == *other.d)
            && val == other.val);
}

/*!
    Destroys the object and frees any allocated resources.
*/

QSqlField::~QSqlField()
{
    if (!--d->ref)
        delete d;
}

/*!
    Sets the value of the field to \a value. If the field is read-only
    (isReadOnly() returns true), nothing happens. If the data type of
    \a value differs from the field's current data type, an attempt is
    made to cast it to the proper type. This preserves the data type
    of the field in the case of assignment, e.g. a QString to an
    integer data type. For example:

    \code
    QSqlCursor cur("Employee");                 // 'Employee' table
    QSqlField* f = cur.field("student_count");  // an integer field
    ...
    f->setValue(myLineEdit->text());                  // cast the line edit text to an integer
    \endcode

    \sa isReadOnly()
*/

void QSqlField::setValue(const QCoreVariant& value)
{
    if (isReadOnly())
        return;
    if (value.type() != d->type) {
        if (!val.canCast(d->type))
             qWarning("QSqlField::setValue: %s cannot cast from %s to %s",
                      d->nm.local8Bit(), value.typeName(), QCoreVariant::typeToName(d->type));
    }
    detach();
    val = value;
}

/*!
    Clears the value of the field and sets it to NULL.
    If the field is read-only, nothing happens.
*/

void QSqlField::clear()
{
    if (isReadOnly())
        return;
    detach();
    val = QCoreVariant(type());
}

/*!
    \fn void QSqlField::setName(const QString& name)

    Sets the name of the field to \a name.
*/

void QSqlField::setName(const QString& name)
{
    detach();
    d->nm = name;
}

/*!
    \fn void QSqlField::setNull()
    \obsolete

    use clear() instead.

    \sa clear().
*/

/*!
    \fn void QSqlField::setReadOnly(bool readOnly)

    Sets the read only flag of the field's value to \a readOnly.

    \sa setValue()
*/
void QSqlField::setReadOnly(bool readOnly)
{
    detach();
    d->ro = readOnly;
}

/*! \fn QCoreVariant QSqlField::value() const
    Returns the value of the field as a QCoreVariant.
*/

/*!
    Returns the name of the field.
*/
QString QSqlField::name() const
{
    return d->nm;
}

/*!
    Returns the field's type.
*/
QCoreVariant::Type QSqlField::type() const
{
    return d->type;
}

void QSqlField::setType(QCoreVariant::Type type)
{
    detach();
    d->type = type;
}


/*!
    Returns true if the field's value is read only; otherwise returns
    false.
*/
bool QSqlField::isReadOnly() const
{ return d->ro; }

/*!
    Returns true if the field is currently NULL; otherwise returns
    false.
*/
bool QSqlField::isNull() const
{ return val.isNull(); }

/*! \internal
*/
void QSqlField::detach()
{
    if (d->ref == 1)
        return;

    QSqlFieldPrivate *x = new QSqlFieldPrivate(*d);
    x = qAtomicSetPtr(&d, x);
    if (!--x->ref)
        delete x;
}

int QSqlField::isRequired() const
{
    return d->req;
}

int QSqlField::length() const
{
    return d->len;
}

int QSqlField::precision() const
{
    return d->pr;
}

QCoreVariant QSqlField::defaultValue() const
{
    return d->def;
}

int QSqlField::typeID() const
{
    return d->tp;
}

int QSqlField::isAutoGenerated() const
{
    return d->gen;
}

bool QSqlField::isValid() const
{
    return d->type != QCoreVariant::Invalid;
}


#if !defined(Q_OS_MAC) || QT_MACOSX_VERSION >= 0x1030
#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug dbg, const QSqlField &f)
{
    dbg.nospace() << "QSqlField(\"" << f.name() << "\", " << QCoreVariant::typeToName(f.type());
    if (f.length() >= 0)
        dbg.nospace() << ", length: " << f.length();
    if (f.precision() >= 0)
        dbg.nospace() << ", precision: " << f.precision();
    if (f.isRequired() >= 0)
        dbg.nospace() << ", required: " << f.isRequired();
    if (f.isAutoGenerated() >= 0)
        dbg.nospace() << ", auto-generated: " << f.isAutoGenerated();
    if (f.typeID() >= 0)
        dbg.nospace() << ", typeID: " << f.typeID();
    if (!f.defaultValue().isNull())
        dbg.nospace() << ", auto-value: \"" << f.defaultValue() << "\"";
    dbg.nospace() << ")";
    return dbg.space();
}
#endif
#endif

#endif
