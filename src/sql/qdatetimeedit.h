/****************************************************************************
**
** Definition of date and time edit classes
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

#ifndef QDATETIMEEDIT_H
#define QDATETIMEEDIT_H

#include "qfeatures.h"

#if !defined( QT_MODULE_SQL ) || defined( QT_LICENSE_PROFESSIONAL )
#define QM_EXPORT_SQL
#else
#define QM_EXPORT_SQL Q_EXPORT
#endif

#ifndef QT_NO_SQL

#ifndef QT_H
#include "qwidget.h"
#include "qstring.h"
#include "qdatetime.h"
#endif // QT_H

class QDateTimeEditBase : public QWidget
{
    Q_OBJECT
public:
    QDateTimeEditBase( QWidget * parent = 0, const char * name = 0 )
	: QWidget( parent, name ) {}
    
public slots:
    virtual void stepUp() {};
    virtual void stepDown() {};

public: //protected:
    virtual bool setFocusSection( int /*s*/ ){ return TRUE; };
    virtual QString sectionFormattedText( int /*sec*/ ){ return QString(); };
    virtual void addNumber( int /*sec*/, int /*num*/ ){};
    virtual void removeLastNumber( int /*sec*/ ){};
};

class QDateEditPrivate;

class QM_EXPORT_SQL QDateEdit : public QDateTimeEditBase
{
    Q_OBJECT
    Q_ENUMS( Order )
    Q_PROPERTY( Order order READ order WRITE setOrder )
    Q_PROPERTY( QDate date READ date WRITE setDate )
    Q_PROPERTY( bool autoAdvance READ autoAdvance WRITE setAutoAdvance )
    Q_PROPERTY( QDate maxValue READ maxValue WRITE setMaxValue )
    Q_PROPERTY( QDate minValue READ minValue WRITE setMinValue )

public:
    QDateEdit( QWidget * parent = 0,  const char * name = 0 );
    QDateEdit( const QDate& date, QWidget * parent = 0,  const char * name = 0 );
    ~QDateEdit();

    enum Order {
	DMY,
	MDY,
	YMD,
	YDM
    };

    QSize sizeHint() const;
    virtual void setDate( const QDate& date );
    QDate date() const;
    virtual void setOrder( Order order );
    Order order() const;
    virtual void setAutoAdvance( bool advance );
    bool autoAdvance() const;

    virtual void setMinValue( const QDate& d ) { setRange( d, maxValue() ); }
    QDate minValue() const;
    virtual void setMaxValue( const QDate& d ) { setRange( minValue(), d ); }
    QDate maxValue() const;
    virtual void setRange( const QDate& min, const QDate& max );

    bool frame() const { return TRUE; }
    void setFrame( const bool /*b*/ ) {}

signals:
    void valueChanged( const QDate& date );

protected:
    bool event( QEvent *e );
    void timerEvent ( QTimerEvent * );
    void resizeEvent ( QResizeEvent * );
    void stepUp();
    void stepDown();
    QString sectionFormattedText( int sec );
    void addNumber( int sec, int num );
    void removeLastNumber( int sec );
    bool setFocusSection( int s );
    
    virtual void setYear( int year );
    virtual void setMonth( int month );
    virtual void setDay( int day );
    virtual void fix();
    virtual bool outOfRange( int y, int m, int d ) const;

private:
    void init();
    int sectionOffsetEnd( int sec );
    int sectionLength( int sec );
    QString sectionText( int sec );
    QDateEditPrivate* d;
};

class QTimeEditPrivate;

class QM_EXPORT_SQL QTimeEdit : public QDateTimeEditBase
{
    Q_OBJECT
    Q_PROPERTY( QTime time READ time WRITE setTime )
    Q_PROPERTY( bool autoAdvance READ autoAdvance WRITE setAutoAdvance )
    Q_PROPERTY( QTime maxValue READ maxValue WRITE setMaxValue )
    Q_PROPERTY( QTime minValue READ minValue WRITE setMinValue )

public:
    QTimeEdit( QWidget * parent = 0,  const char * name = 0 );
    QTimeEdit( const QTime& time, QWidget * parent = 0,  const char * name = 0 );
    ~QTimeEdit();

    QSize sizeHint() const;
    virtual void setTime( const QTime& time );
    QTime time() const;
    virtual void setAutoAdvance( bool advance );
    bool autoAdvance() const;

    virtual void setMinValue( const QTime& d ) { setRange( d, maxValue() ); }
    QTime minValue() const;
    virtual void setMaxValue( const QTime& d ) { setRange( minValue(), d ); }
    QTime maxValue() const;
    virtual void setRange( const QTime& min, const QTime& max );

signals:
    void valueChanged( const QTime& time );

protected:
    bool event( QEvent *e );
    void timerEvent ( QTimerEvent *e );
    void resizeEvent ( QResizeEvent * );
    void stepUp();
    void stepDown();
    QString sectionFormattedText( int sec );
    void addNumber( int sec, int num );
    void removeLastNumber( int sec );
    bool setFocusSection( int s );
    virtual bool outOfRange( int h, int m, int s ) const;

    virtual void setHour( int h );
    virtual void setMinute( int m );
    virtual void setSecond( int s );

private:
    void init();
    QString sectionText( int sec );
    QTimeEditPrivate* d;
};

class QDateTimeEditPrivate;

class QM_EXPORT_SQL QDateTimeEdit : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( QDateTime dateTime READ dateTime WRITE setDateTime )

public:
    QDateTimeEdit( QWidget * parent = 0, const char * name = 0 );
    QDateTimeEdit( const QDateTime& datetime, QWidget * parent = 0,
		   const char * name = 0 );
    ~QDateTimeEdit();
    QSize sizeHint() const;
    virtual void  setDateTime( const QDateTime & dt );
    QDateTime dateTime() const;

    QDateEdit* dateEdit() { return de; }
    QTimeEdit* timeEdit() { return te; }

    virtual void setAutoAdvance( bool advance );
    bool autoAdvance() const;

signals:
    void valueChanged( const QDateTime& datetime );

protected:
    void init();
    void resizeEvent( QResizeEvent * );
    QSize minimumSizeHint() const;
    void layoutEditors();

protected slots:
    void newValue( const QDate& d );
    void newValue( const QTime& t );

private:
    QDateEdit* de;
    QTimeEdit* te;
    QDateTimeEditPrivate* d;
};


#endif
#endif
