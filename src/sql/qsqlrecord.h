/****************************************************************************
**
** Definition of QSqlRecord class.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
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

#if !defined( QT_MODULE_SQL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_EXPORT_SQL
#else
#define QM_EXPORT_SQL Q_SQL_EXPORT
#endif

class QM_EXPORT_SQL QSqlRecord
{
public:
    QSqlRecord();
    QSqlRecord( const QSqlRecord& other );
    QSqlRecord& operator=( const QSqlRecord& other );
    virtual ~QSqlRecord();
    virtual QCoreVariant value( int i ) const;
    QCoreVariant value( const QString& name ) const;
    virtual void setValue( int i, const QCoreVariant& val );
    void setValue( const QString& name, const QCoreVariant& val );
    bool isGenerated( int i ) const;
    bool isGenerated( const QString& name ) const;
    void setGenerated( const QString& name, bool generated );
    virtual void setGenerated( int i, bool generated );
    virtual void setNull( int i );
    void setNull( const QString& name );
    bool isNull( int i ) const;
    bool isNull( const QString& name ) const;

    int position( const QString& name ) const;
    QString fieldName( int i ) const;
    QSqlField* field( int i );
    QSqlField* field( const QString& name );
    const QSqlField* field( int i ) const;
    const QSqlField* field( const QString& name ) const;

    virtual void append(const QSqlField& field);
    virtual void replace(int pos, const QSqlField& field);
    virtual void remove(int pos);

#ifdef QT_COMPAT
    inline QT_COMPAT void insert(int pos, const QSqlField& field) { replace(pos, field); }
#endif

    bool isEmpty() const;
    bool contains( const QString& name ) const;
    virtual void clear();
    virtual void clearValues();
    int count() const;
    virtual QString toString( const QString& prefix = QString(),
				   const QString& sep = "," ) const;
    virtual QStringList toStringList( const QString& prefix = QString() ) const;

private:
    void detach();
    QSqlRecordPrivate* d;
};

#ifndef QT_NO_DEBUG
QDebug operator<<(QDebug, const QSqlRecord &);
#endif

#endif	// QT_NO_SQL
#endif
