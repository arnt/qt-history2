/****************************************************************************
**
** Definition of QSqlError class.
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

#ifndef QSQLERROR_H
#define QSQLERROR_H

#ifndef QT_H
#include "qstring.h"
#endif // QT_H

#if !defined( QT_MODULE_SQL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_EXPORT_SQL
#else
#define QM_EXPORT_SQL Q_SQL_EXPORT
#endif

#ifndef QT_NO_SQL

class QM_EXPORT_SQL QSqlError
{
public:
    enum Type {
	None,
	Connection,
	Statement,
	Transaction,
	Unknown
    };
    QSqlError(  const QString& driverText = QString(),
		const QString& databaseText = QString(),
		int type = QSqlError::None,
		int number = -1 );
    QSqlError( const QSqlError& other );
    QSqlError& operator=( const QSqlError& other );
    virtual ~QSqlError();

    QString	driverText() const;
    virtual void setDriverText( const QString& driverText );
    QString	databaseText() const;
    virtual void setDatabaseText( const QString& databaseText );
    int		type() const;
    virtual void setType( int type );
    int		number() const;
    virtual void setNumber( int number );
    QString 	text() const;

private:
    QString	driverError;
    QString	databaseError;
    int		errorType;
    int	errorNumber;
};

#ifndef QT_NO_DEBUG
QM_EXPORT_SQL QDebug operator<<(QDebug, const QSqlError &);
#endif

#endif // QT_NO_SQL
#endif
