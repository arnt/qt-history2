/****************************************************************************
**
** Definition of QSqlRecord class.
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

#ifndef QSQLRECORD_H
#define QSQLRECORD_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlField;
class QStringList;
class QCoreVariant;
class QSqlRecordPrivate;

#if defined(QT_LICENSE_PROFESSIONAL)
#define QM_EXPORT_SQL
#else
#define QM_EXPORT_SQL Q_SQL_EXPORT
#endif

class QM_EXPORT_SQL QSqlRecord
{
public:
    QSqlRecord();
    QSqlRecord(const QSqlRecord& other);
    QSqlRecord& operator=(const QSqlRecord& other);
    ~QSqlRecord();

    QCoreVariant value(int i) const;
    QCoreVariant value(const QString& name) const;
    void setValue(int i, const QCoreVariant& val);
    void setValue(const QString& name, const QCoreVariant& val);

    void setNull(int i);
    void setNull(const QString& name);
    bool isNull(int i) const;
    bool isNull(const QString& name) const;

    int indexOf(const QString &name) const;
    QString fieldName(int i) const;

    QSqlField field(int i) const;
    QSqlField field(const QString &name) const;

    bool isGenerated(int i) const;
    bool isGenerated(const QString& name) const;
    void setGenerated(const QString& name, bool generated);
    void setGenerated(int i, bool generated);

#ifdef QT_COMPAT
    QT_COMPAT const QSqlField* fieldPtr(int i) const;
    QT_COMPAT const QSqlField* fieldPtr(const QString& name) const;
    inline QT_COMPAT int position(const QString& name) const { return indexOf(name); }
    QT_COMPAT QString toString(const QString& prefix = QString(),
                               const QString& sep = QLatin1String(",")) const;
    QT_COMPAT QStringList toStringList(const QString& prefix = QString()) const;
#endif

    void append(const QSqlField& field);
    void replace(int pos, const QSqlField& field);
    void insert(int pos, const QSqlField& field);
    void remove(int pos);

    bool isEmpty() const;
    bool contains(const QString& name) const;
    void clear();
    void clearValues();
    int count() const;

private:
    void detach();
    QSqlRecordPrivate* d;
};

#ifndef QT_NO_DEBUG
QM_EXPORT_SQL QDebug operator<<(QDebug, const QSqlRecord &);
#endif

#endif        // QT_NO_SQL
#endif
