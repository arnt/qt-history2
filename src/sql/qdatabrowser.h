/****************************************************************************
**
** Definition of QDataBrowser class
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

#ifndef QDATABROWSER_H
#define QDATABROWSER_H

#include "qfeatures.h"

#ifndef QT_NO_SQL

#ifndef QT_H
#include "qdatahandler.h"
#include "qwidget.h"
#include "qsqlnavigator.h"
#include "qstring.h"
#include "qstringlist.h"
#endif // QT_H

class Q_EXPORT QDataBrowser : public QWidget, public QDataHandler
{
    Q_OBJECT
    Q_PROPERTY( bool boundryChecking READ boundryChecking WRITE setBoundryChecking )
    Q_PROPERTY( QString filter READ filter WRITE setFilter )
    Q_PROPERTY( QStringList sort READ sort WRITE setSort )
    Q_PROPERTY( bool autoEdit READ autoEdit WRITE setAutoEdit )

public:
    QDataBrowser( QWidget *parent = 0, const char *name = 0, WFlags fl = 0 );
    ~QDataBrowser();

    enum Boundry {
	Unknown,
	None,
	BeforeBeginning,
	Beginning,
	End,
	AfterEnd
    };

    Boundry boundry();
    void setBoundryChecking( bool active );
    bool boundryChecking() const;

    void setSort( const QSqlIndex& sort );
    void setSort( const QStringList& sort );
    QStringList  sort() const;
    void setFilter( const QString& filter );
    QString filter() const;
    virtual void setCursor( QSqlCursor* cursor, bool autoDelete = FALSE );
    void setSqlCursor( QSqlCursor* cursor, bool autoDelete = FALSE );
    QSqlCursor* sqlCursor() const;
    virtual void setForm( QSqlForm* form );
    QSqlForm* form();

    void setAutoEdit( bool autoEdit );
    bool autoEdit() const;

signals:
    void firstRecordAvailable( bool available );
    void lastRecordAvailable( bool available );
    void nextRecordAvailable( bool available );
    void prevRecordAvailable( bool available );

    void currentChanged( const QSqlRecord* record );
    void primeInsert( QSqlRecord* buf );
    void primeUpdate( QSqlRecord* buf );
    void primeDelete( QSqlRecord* buf );
    void cursorChanged( QSqlCursor::Mode mode );

public slots:
    virtual void refresh();

    virtual void insert();
    virtual void update();
    virtual void del();

    virtual void first();
    virtual void last();
    virtual void next();
    virtual void prev();

    virtual void readFields();
    virtual void writeFields();
    virtual void clearValues();

protected:
    virtual bool insertCurrent();
    virtual bool updateCurrent();
    virtual bool deleteCurrent();
    virtual bool currentEdited();

private:
    void updateBoundry();
    class QDataBrowserPrivate;
    QDataBrowserPrivate* d;
};


#endif
#endif
