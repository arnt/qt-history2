/****************************************************************************
**
** Definition of QSqlCursor class
**
** Created : 001103
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
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

#ifndef QSQLCURSOR_H
#define QSQLCURSOR_H

#ifndef QT_H
#include "qstring.h"
#include "qstringlist.h"
#include "qsqlrecord.h"
#include "qsqlquery.h"
#include "qsqlindex.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlDatabase;
class QSqlCursorPrivate;

class Q_EXPORT QSqlCursor : public QSqlRecord, public QSqlQuery
{
public:
    QSqlCursor( const QString & name = QString::null, bool autopopulate = TRUE, QSqlDatabase* db = 0 );
    QSqlCursor( const QSqlCursor & other );
    QSqlCursor& operator=( const QSqlCursor& other );
    ~QSqlCursor();

    enum Mode {
	ReadOnly = 0,
	Insert = 1,
	Update = 2,
	Delete = 4,
	Writable = 7
    };

    QVariant          value( int i ) const;
    QVariant          value( const QString& name ) const;
    virtual QSqlIndex primaryIndex( bool prime = FALSE ) const;
    virtual QSqlIndex index( const QStringList& fieldNames ) const;
    QSqlIndex         index( const QString& fieldName ) const;
    QSqlIndex         index( const char* fieldName ) const;
    virtual void      setPrimaryIndex( const QSqlIndex& idx );

    virtual QSqlRecord* insertBuffer( bool clearValues = TRUE, bool prime = TRUE );
    virtual int         insert( bool invalidate = TRUE );
    virtual QSqlRecord* updateBuffer( bool copyCursor = TRUE, bool prime = TRUE );
    virtual int         update( bool invalidate = TRUE );
    virtual int         del( bool invalidate = TRUE );

    virtual void      setMode( int flags );
    int               mode() const;
    virtual void      setCalculated( const QString& name, bool calculated );
    bool              isCalculated( const QString& name ) const;
    bool              isReadOnly() const;
    bool              canInsert() const;
    bool              canUpdate() const;
    bool              canDelete() const;

    bool              select();
    bool              select( const QSqlIndex& sort );
    bool              select( const QSqlIndex & filter, const QSqlIndex & sort );
    virtual bool      select( const QString & filter, const QSqlIndex & sort = QSqlIndex() );
    
    virtual void      setSort( const QSqlIndex& sort );
    QSqlIndex         sort() const;
    virtual void      setFilter( const QString& filter );
    QString           filter() const;
    virtual void      setName( const QString& name, bool autopopulate = TRUE );
    QString           name() const;
    QString           toString( const QString& prefix = QString::null ) const;

protected:
    void              afterSeek();
    bool              exec( const QString & sql );

    virtual void      primeInsert( QSqlRecord* buf );
    virtual void      primeUpdate( QSqlRecord* buf );

    virtual QVariant  calculateField( const QString& name );
    virtual int       update( const QString & filter, bool invalidate = TRUE );
    virtual int       del( const QString & filter, bool invalidate = TRUE );
    
    virtual QString   toString( const QString& prefix, QSqlField* field, const QString& fieldSep ) const;
    virtual QString   toString( QSqlRecord* rec, const QString& prefix, const QString& fieldSep, 
				const QString& sep ) const;
    virtual QString   toString( const QSqlIndex& i, QSqlRecord* rec, const QString& prefix, 
				const QString& fieldSep, const QString& sep ) const;

private:
    void              sync();
    int               apply( const QString& q, bool invalidate );
    QSqlRecord&       operator=( const QSqlRecord & list );
    
    QSqlCursorPrivate*  d;
};




#endif	// QT_NO_SQL
#endif
