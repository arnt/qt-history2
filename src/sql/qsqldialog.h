/****************************************************************************
**
** Definition of QSqlDialog class
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


#ifndef QSQLDIALOG_H
#define QSQLDialog_H

#ifndef QT_NO_SQL

#ifndef QT_H
#include "qdialog.h"
#include "qsqlnavigator.h"
#include "qstring.h"
#include "qstringlist.h"
#endif // QT_H

class Q_EXPORT QSqlDialog : public QDialog, public QSqlFormNavigator
{
    Q_OBJECT
    Q_PROPERTY( bool boundryChecking READ boundryChecking WRITE setBoundryChecking )
    Q_PROPERTY( QString filter READ filter WRITE setFilter )
    Q_PROPERTY( QStringList sort READ sort WRITE setSort )

public:
    QSqlDialog( QWidget *parent = 0, const char *name = 0, bool modal = FALSE, WFlags f = 0 );

    virtual void setBoundryChecking( bool active );
    bool boundryChecking() const;

    virtual void setSort( const QSqlIndex& sort );
    virtual void setSort( const QStringList& sort );
    QStringList  sort() const;
    virtual void setFilter( const QString& filter );
    QString filter() const;

signals:
    void firstRecordAvailable( bool available );
    void lastRecordAvailable( bool available );
    void nextRecordAvailable( bool available );
    void prevRecordAvailable( bool available );

public slots:
    virtual void insertRecord();
    virtual void updateRecord();
    virtual void deleteRecord();
    virtual void firstRecord();
    virtual void lastRecord();
    virtual void nextRecord();
    virtual void prevRecord();
    virtual void clearForm();

protected:
    void emitFirstRecordAvailable( bool available );
    void emitLastRecordAvailable( bool available );
    void emitNextRecordAvailable( bool available );
    void emitPrevRecordAvailable( bool available );

};

#endif // QT_NO_SQL

#endif
