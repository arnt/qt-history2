/****************************************************************************
**
** Definition of QSqlWidget class
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

#ifndef QSQLWIDGET_H
#define QSQLWIDGET_H

#include "qfeatures.h"

#ifndef QT_NO_SQL

#ifndef QT_H
#include "qwidget.h"
#include "qsqlnavigator.h"
#include "qstring.h"
#include "qstringlist.h"
#endif // QT_H

class Q_EXPORT QSqlWidget : public QWidget, public QSqlFormNavigator
{
    Q_OBJECT
    Q_PROPERTY( bool boundryChecking READ boundryChecking WRITE setBoundryChecking )
    Q_PROPERTY( QString filter READ filter WRITE setFilter )
    Q_PROPERTY( QStringList sort READ sort WRITE setSort )

public:
    QSqlWidget( QWidget *parent = 0, const char *name = 0, WFlags fl = 0 );

    virtual void setBoundryChecking( bool active );
    bool boundryChecking() const;

    virtual void setSort( const QSqlIndex& sort );
    virtual void setSort( const QStringList& sort );
    QStringList  sort() const;
    virtual void setFilter( const QString& filter );
    QString filter() const;

    void setCursor( QSqlCursor* cursor, bool autoDelete = FALSE ) { QSqlFormNavigator::setCursor( cursor, autoDelete ); }
    void setCursor ( const QCursor & cursor ) { QWidget::setCursor( cursor ); }
    const QCursor& cursor () const { return QWidget::cursor(); }
    void setSqlCursor( QSqlCursor* cursor, bool autoDelete = FALSE ) { QSqlFormNavigator::setCursor( cursor, autoDelete ); }
    QSqlCursor* sqlCursor() const { return QSqlFormNavigator::cursor(); }

signals:
    void firstRecordAvailable( bool available );
    void lastRecordAvailable( bool available );
    void nextRecordAvailable( bool available );
    void prevRecordAvailable( bool available );

    void currentChanged( const QSqlRecord* record );
    void primeInsert( QSqlRecord* buf );
    void primeUpdate( QSqlRecord* buf );
    void beforeInsert( QSqlRecord* buf );
    void beforeUpdate( QSqlRecord* buf );
    void beforeDelete( QSqlRecord* buf );
    void cursorChanged( QSqlCursor::Mode mode );

public slots:
    virtual void insertRecord();
    virtual void updateRecord();
    virtual void deleteRecord();
    virtual void firstRecord();
    virtual void lastRecord();
    virtual void nextRecord();
    virtual void prevRecord();
    virtual void clearFormValues();

    virtual void readFields();
    virtual void writeFields();

protected:
    void emitCurrentChanged( const QSqlRecord* record );
    void emitFirstRecordAvailable( bool available );
    void emitLastRecordAvailable( bool available );
    void emitNextRecordAvailable( bool available );
    void emitPrevRecordAvailable( bool available );
    void emitBeforeInsert( QSqlRecord* buf );
    void emitBeforeUpdate( QSqlRecord* buf );
    void emitBeforeDelete( QSqlRecord* buf );
    void emitCursorChanged( QSqlCursor::Mode mode );
};


#endif
#endif
