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

#ifndef QT_NO_SQL

#ifndef QT_H
#include "qwidget.h"
#include "qstring.h"
#include "qdatetime.h"
#include "qlineedit.h"
#include "qframe.h"
#include "qtimer.h"
#endif // QT_H

class QPushButton;


//## todo multiple ctor taking data types

class Q_EXPORT QNumberSection
{
public:
    QNumberSection( int selStart = 0, int selEnd = 0 )
	: selstart( selStart ), selend( selEnd )
    {}
    int selectionStart() const { return selstart; }
    void setSelectionStart( int s ) { selstart = s; }
    int selectionEnd() const { return selend; }
    void setSelectionEnd( int s ) { selend = s; }
    int width() const { return selend - selstart; }
private:
    int selstart;
    int selend;
};

class Q_EXPORT QDateTimeEditBase : public QWidget
{
    Q_OBJECT
    Q_PROPERTY( bool frame READ frame WRITE setFrame )

public:
    QDateTimeEditBase( QWidget * parent = 0,
		       const char * name = 0 );
    ~QDateTimeEditBase();


    bool frame() const;
    void setSeparator( const QString& s );
    QString separator() const;

public slots:
    virtual void stepUp();
    virtual void stepDown();
    virtual void setFrame( bool );

signals:
    void valueChanged();

protected:
    void init();
    bool event( QEvent *e );
    void resizeEvent( QResizeEvent * );
    void paintEvent( QPaintEvent * );
    void mousePressEvent( QMouseEvent *e );
    void keyPressEvent( QKeyEvent *e );
    void updateArrows();
    void layoutArrows( const QSize& s );
    int focusSection() const;

    void setSectionSelection( int sec, int selstart, int selend );
    void appendSection( const QNumberSection& sec );
    virtual bool setFocusSection( int s );
    virtual QString sectionFormattedText( int sec );
    virtual void addNumber( int sec, int num );
    virtual void removeLastNumber( int sec );

private:
    QPushButton        * up, * down;
    class QDateTimeEditBasePrivate;
    QDateTimeEditBasePrivate* d;
};

class Q_EXPORT QDateEdit : public QDateTimeEditBase
{
    Q_OBJECT
    Q_ENUMS( Order )
    Q_PROPERTY( Order order READ order WRITE setOrder )
    Q_PROPERTY( QDate date READ date WRITE setDate )
    Q_PROPERTY( bool autoAdvance READ autoAdvance WRITE setAutoAdvance )

public:
    QDateEdit( QWidget * parent = 0,  const char * name = 0 );
    ~QDateEdit();

    enum Order {
	DMY,
	MDY,
	YMD,
	YDM
    };

    QSize sizeHint() const;
    void setDate( const QDate& date );
    QDate date() const;
    void setOrder( Order order );
    Order order() const;
    void setAutoAdvance( bool advance );
    bool autoAdvance() const;

signals:
    void valueChanged( const QDate& );

protected:
    bool event( QEvent *e );
    void timerEvent ( QTimerEvent *e );
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

private:
    int sectionOffsetEnd( int sec );
    int sectionLength( int sec );
    QString sectionText( int sec );
    class QDateEditPrivate;
    QDateEditPrivate* d;
};

class Q_EXPORT QTimeEdit : public QDateTimeEditBase
{
    Q_OBJECT
    Q_PROPERTY( QTime time READ time WRITE setTime )
    Q_PROPERTY( bool autoAdvance READ autoAdvance WRITE setAutoAdvance )

public:
    QTimeEdit( QWidget * parent = 0,  const char * name = 0 );
    ~QTimeEdit();

    QSize sizeHint() const;
    void setTime( const QTime& time );
    QTime time() const;
    void setAutoAdvance( bool advance );
    bool autoAdvance() const;

signals:
    void valueChanged( const QTime& );

protected:
    bool event( QEvent *e );
    void timerEvent ( QTimerEvent *e );
    void stepUp();
    void stepDown();
    QString sectionFormattedText( int sec );
    void addNumber( int sec, int num );
    void removeLastNumber( int sec );
    bool setFocusSection( int s );

    void setHour( int h );
    void setMinute( int m );
    void setSecond( int s );
    QString sectionText( int sec );

private:
    class QTimeEditPrivate;
    QTimeEditPrivate* d;
};


class Q_EXPORT QDateTimeEdit : public QFrame
{
    Q_OBJECT
    Q_PROPERTY( QDateTime dateTime READ dateTime WRITE setDateTime )
public:
    QDateTimeEdit( QWidget * parent = 0, const char * name = 0 );
    ~QDateTimeEdit();
    QSize sizeHint() const;
    void  setDateTime( const QDateTime & dt );
    QDateTime dateTime() const;

    void    setTimeSeparator( const QString & separator );
    QString timeSeparator() const;
    void    setDateSeparator( const QString & separator );
    QString dateSeparator() const;

    void setAutoAdvance( bool advance );
    bool autoAdvance() const;

signals:
    void valueChanged( const QDateTime& );

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
    class QDateTimeEditPrivate;
    QDateTimeEditPrivate* d;
};


#endif
#endif
