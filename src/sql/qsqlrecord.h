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
#include "qshared.h"
#include "qsqlfield.h"
#include "qstringlist.h"
#include "qvariant.h"
#endif // QT_H

#ifndef QT_NO_SQL

template <typename T> class QList;
class QSqlRecordPrivate;

class QSqlRecordShared : public QShared
{
public:
    QSqlRecordShared( QSqlRecordPrivate* sqlRecordPrivate )
    : d( sqlRecordPrivate )
    {}
    virtual ~QSqlRecordShared();
    QSqlRecordPrivate* d;
};

class Q_EXPORT QSqlRecord
{
public:
    QSqlRecord();
    QSqlRecord( const QSqlRecord& other );
    QSqlRecord& operator=( const QSqlRecord& other );
    virtual ~QSqlRecord();
    virtual QVariant     value( int i ) const;
    virtual QVariant     value( const QString& name ) const;
    virtual void         setValue( int i, const QVariant& val );
    virtual void         setValue( const QString& name, const QVariant& val );
    bool                 isGenerated( int i ) const;
    bool                 isGenerated( const QString& name ) const;
    virtual void         setGenerated( const QString& name, bool generated );
    virtual void         setGenerated( int i, bool generated );
    virtual void         setNull( int i );
    virtual void         setNull( const QString& name );
    bool                 isNull( int i ) const;
    bool                 isNull( const QString& name ) const;

    int                  position( const QString& name ) const;
    QString              fieldName( int i ) const;
    QSqlField*           field( int i );
    QSqlField*           field( const QString& name );
    const QSqlField*     field( int i ) const;
    const QSqlField*     field( const QString& name ) const;

    virtual void         append( const QSqlField& field );
    virtual void         insert( int pos, const QSqlField& field );
    virtual void         remove( int pos );

    bool                 isEmpty() const;
    bool                 contains( const QString& name ) const;
    virtual void         clear();
    virtual void         clearValues( bool nullify = FALSE );
    int                 count() const;
    virtual QString      toString( const QString& prefix = QString(),
				   const QString& sep = "," ) const;
    virtual QStringList  toStringList( const QString& prefix = QString() ) const;

private:
    QString              createField( int i, const QString& prefix ) const;
    void                 deref();
    bool                 checkDetach();
    QSqlRecordShared*    sh;
};

/******************************************/
/*******     QSqlRecordInfo Class    ******/
/******************************************/

#if defined(Q_TEMPLATEDLL)
// MOC_SKIP_BEGIN
Q_TEMPLATE_EXTERN template class Q_EXPORT QList<QSqlFieldInfo>;
// MOC_SKIP_END
#endif

typedef QList<QSqlFieldInfo> QSqlFieldInfoList;

class Q_EXPORT QSqlRecordInfo: public QSqlFieldInfoList
{
public:
    QSqlRecordInfo(): QSqlFieldInfoList() {}
    QSqlRecordInfo( const QSqlFieldInfoList& other ): QSqlFieldInfoList( other ) {}
    QSqlRecordInfo( const QSqlRecord& other );

    size_type contains( const QString& fieldName ) const;
    QSqlFieldInfo find( const QString& fieldName ) const;
    QSqlRecord toRecord() const;

};


#endif	// QT_NO_SQL
#endif
