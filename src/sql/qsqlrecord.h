/****************************************************************************
**
** Definition of QSqlRecord class
**
** Created : 2000-11-03
**
** Copyright (C) 2000 Trolltech AS.  All rights reserved.
**
** This file is part of the sql module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition licenses may use this
** file in accordance with the Qt Commercial License Agreement provided
** with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QSQLRECORD_H
#define QSQLRECORD_H

#include "qfeatures.h"

#ifndef QT_NO_SQL

#ifndef QT_H
#include "qstring.h"
#include "qstringlist.h"
#include "qvariant.h"
#include "qsqlfield.h"
#endif // QT_H

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
    uint                 count() const;
    virtual QString      toString( const QString& prefix = QString::null,
				   const QString& sep = "," ) const;
    virtual QStringList  toStringList( const QString& prefix = QString::null ) const;

    virtual void         setGenerated( const QString& name, bool generated );
    bool                 isGenerated( const QString& name ) const;

private:
    QString              createField( int i, const QString& prefix ) const;
    void                 deref();
    bool                 checkDetach();
    QSqlRecordShared*    sh;
};

#endif	// QT_NO_SQL
#endif
