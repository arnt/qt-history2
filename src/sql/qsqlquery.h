/****************************************************************************
**
** Definition of QSqlQuery class
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

#ifndef QSQLQUERY_H
#define QSQLQUERY_H

#ifndef QT_H
#include "qobject.h"
#include "qstring.h"
#include "qvariant.h"
#include "qvaluelist.h"
#include "qsqlerror.h"
#include "qsqlfield.h"
#endif // QT_H

#ifndef QT_NO_SQL

class QSqlDriver;
class QSqlResult;
class QSqlResultInfo;
class QSqlDatabase;

class Q_EXPORT QSqlResultShared : public QObject, public QShared
{
    Q_OBJECT
public:
    QSqlResultShared( QSqlResult* result );
    virtual ~QSqlResultShared();
    QSqlResult* sqlResult;
private slots:
    void slotResultDestroyed();
};

class Q_EXPORT QSqlQuery
{
public:
    QSqlQuery( QSqlResult * r );
    QSqlQuery( const QString& query = QString::null, QSqlDatabase* db = 0 );
    QSqlQuery( const QSqlQuery& other );
    QSqlQuery&          operator=( const QSqlQuery& other );
    virtual ~QSqlQuery();

    bool                isValid() const;
    bool                isActive() const;
    bool	        isNull( int field ) const;
    int                 at() const;
    QString             lastQuery() const;
    int                 numRowsAffected() const;
    QSqlError	        lastError() const;
    bool                isSelect() const;
    int                 size() const;
    const QSqlDriver*   driver() const;
    const QSqlResult*   result() const;
    bool		isForwardOnly() const;
    void		setForwardOnly( bool forward );

    virtual bool	exec ( const QString& query );
    virtual QVariant    value( int i ) const;

    virtual bool	seek( int i, bool relative = FALSE );
    virtual bool        next();
    virtual bool        prev();
    virtual bool        first();
    virtual bool        last();

protected:
    virtual void        beforeSeek();
    virtual void        afterSeek();

private:
    void                deref();
    bool                checkDetach();
    QSqlResultShared*   d;
};


#endif // QT_NO_SQL
#endif
