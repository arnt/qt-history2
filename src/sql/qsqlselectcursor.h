/****************************************************************************
**
** Definition of QSqlSelectCursor class.
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

#ifndef QSQLSELECTCURSOR_H
#define QSQLSELECTCURSOR_H

#ifndef QT_H
#include "qsqlcursor.h"
#endif // QT_H

#if !defined( QT_MODULE_SQL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_EXPORT_SQL
#else
#define QM_EXPORT_SQL Q_EXPORT
#endif

#ifndef QT_NO_SQL

class QSqlSelectCursorPrivate;

class QM_EXPORT_SQL QSqlSelectCursor : public QSqlCursor
{
public:
    QSqlSelectCursor( const QString& query = QString(), QSqlDatabase* db = 0 );
    QSqlSelectCursor( const QSqlSelectCursor& other );
    ~QSqlSelectCursor();
    bool exec( const QString& query );
    bool select() { return QSqlCursor::select(); }
    
protected:
    QSqlIndex primaryIndex( bool = TRUE ) const { return QSqlIndex(); }
    QSqlIndex index( const QStringList& ) const { return QSqlIndex(); }
    QSqlIndex index( const QString& ) const { return QSqlIndex(); }
    QSqlIndex index( const char* ) const { return QSqlIndex(); }
    void setPrimaryIndex( const QSqlIndex& ) {}
    void append( const QSqlFieldInfo& ) {}
    void insert( int, const QSqlFieldInfo& ) {}
    void remove( int ) {}
    void clear() {}
    void setGenerated( const QString&, bool ) {}
    void setGenerated( int, bool ) {}
    QSqlRecord*	editBuffer( bool = FALSE ) { return 0; }
    QSqlRecord*	primeInsert() { return 0; }
    QSqlRecord*	primeUpdate() { return 0; }
    QSqlRecord*	primeDelete() { return 0; }
    int	insert( bool = TRUE ) { return 0; }
    int	update( bool = TRUE ) { return 0; }
    int	del( bool = TRUE ) { return 0; }
    void setMode( int ) {}

    void setSort( const QSqlIndex& ) {}
    QSqlIndex sort() const { return QSqlIndex(); }
    void setFilter( const QString& ) {}
    QString filter() const { return QString(); }
    void setName( const QString&, bool = TRUE ) {}
    QString name() const { return QString(); }
    QString toString( const QString& = QString(), const QString& = "," ) const { return QString(); }
    bool select( const QString &, const QSqlIndex& = QSqlIndex() );

private:
    void populateCursor();
    
    QSqlSelectCursorPrivate * d;
};

#endif // QT_NO_SQL
#endif // QSQLSELECTCURSOR_H
