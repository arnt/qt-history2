/****************************************************************************
**
** Definition of date and time edit classes
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

#ifndef QDATETIMEEDIT_H
#define QDATETIMEEDIT_H

#ifndef QT_H
#include "qwidget.h"
#include "qvalidator.h"
#include "qstring.h"
#include "qdatetime.h"
#include "qlineedit.h"
#include "qframe.h"
#endif // QT_H

#ifndef QT_NO_SQL

class NumEdit;
class QDateTimeEditLabel;

class Q_EXPORT QDateTimeEditBase : public QFrame
{
    Q_OBJECT
public:
    QDateTimeEditBase( QWidget * parent = 0,
		       const char * name = "QDateTimeEditBase" );
    QSize sizeHint() const;

public slots:
    void stepUp();
    void stepDown();

protected:
    void init();
    bool eventFilter( QObject *, QEvent * );
    void updateArrows();
    void layoutArrows();
    void drawContents( QPainter * );

    QPushButton        * up, * down;
    NumEdit            * ed[3];
    QDateTimeEditLabel * sep[2];
    QString lastValid[3];
};


class Q_EXPORT QDateEdit : public QDateTimeEditBase
{
    Q_OBJECT
    Q_PROPERTY( QDate date READ date WRITE setDate )
public:
    QDateEdit( QWidget * parent = 0, const char * name = 0 );
    QDateEdit( const QDate & d, QWidget * parent = 0, const char * name = 0 );
    void    setDate( const QDate & d );
    QDate   date() const;
    void    setOrder( const QString & order );
    QString order() const;

protected:
    void init();
    void fixup();
    void resizeEvent( QResizeEvent * );
    int yearPos, monthPos, dayPos;
    QString format;
};

class Q_EXPORT QTimeEdit : public QDateTimeEditBase
{
    Q_OBJECT
    Q_PROPERTY( QTime time READ time WRITE setTime )
public:
    QTimeEdit( QWidget * parent = 0, const char * name = 0 );
    QTimeEdit( const QTime & d, QWidget * parent = 0, const char * name = 0 );
    void  setTime( const QTime & t );
    QTime time() const;

protected:
    void init();
    void resizeEvent( QResizeEvent * );
};

class Q_EXPORT QDateTimeEdit : public QFrame
{
    Q_OBJECT
    Q_PROPERTY( QDateTime dateTime READ dateTime WRITE setDateTime )
public:
    QDateTimeEdit( QWidget * parent = 0, const char * name = 0 );
    QDateTimeEdit( const QDateTime & dt, QWidget * parent = 0, const char * name = 0 );
    void  setDateTime( const QDateTime & dt );
    QDateTime dateTime() const;

protected:
    void init();
private:
    QDateEdit* de;
    QTimeEdit* te;
};

#endif
#endif
