/****************************************************************************
**
** Definition of QSqlIndex class.
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

#ifndef QSQLINDEX_H
#define QSQLINDEX_H

#ifndef QT_H
#include "qstring.h"
#include "qstringlist.h"
#include "qsqlfield.h"
#include "qsqlrecord.h"
#include "qlist.h"
#endif // QT_H

#if !defined( QT_MODULE_SQL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_EXPORT_SQL
#define QM_TEMPLATE_EXTERN_SQL
#else
#define QM_EXPORT_SQL Q_EXPORT
#define QM_TEMPLATE_EXTERN_SQL Q_TEMPLATE_EXTERN
#endif

#ifndef QT_NO_SQL

#define Q_DEFINED_QSQLINDEX
#include "qwinexport.h"

class QSqlCursor;

class QM_EXPORT_SQL QSqlIndex : public QSqlRecord
{
public:
    QSqlIndex( const QString& cursorName = QString::null, const QString& name = QString::null );
    QSqlIndex( const QSqlIndex& other );
    ~QSqlIndex();
    QSqlIndex&       operator=( const QSqlIndex& other );
    virtual void     setCursorName( const QString& cursorName );
    QString          cursorName() const { return cursor; }
    virtual void     setName( const QString& name );
    QString          name() const { return nm; }

    void             append( const QSqlField& field );
    virtual void     append( const QSqlField& field, bool desc );

    bool             isDescending( int i ) const;
    virtual void     setDescending( int i, bool desc );

    QString          toString( const QString& prefix = QString::null,
			       const QString& sep = ",",
			       bool verbose = TRUE ) const;
    QStringList      toStringList( const QString& prefix = QString::null,
				   bool verbose = TRUE ) const;

    static QSqlIndex fromStringList( const QStringList& l, const QSqlCursor* cursor );

private:
    QString          createField( int i, const QString& prefix, bool verbose ) const;
    QString          cursor;
    QString          nm;
    QList<bool> sorts;
};

#endif	// QT_NO_SQL
#endif
