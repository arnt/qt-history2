#ifndef QDATETIMEEDIT_H
#define QDATETIMEEDIT_H

#ifndef QT_H
#include "qwidget.h"
#include "qvalidator.h"
#include "qstring.h"
#include "qdatetime.h"
#include "qlineedit.h"
#endif // QT_H

#ifndef QT_NO_SQL

class ArrowButton;
class NumEdit;
class QLabel;

class Q_EXPORT QDateTimeEditBase : public QWidget
{
    Q_OBJECT
public:
    QDateTimeEditBase( QWidget * parent = 0, const char * name = 0 );

protected:
    bool eventFilter( QObject *, QEvent * );

    NumEdit     * e[3];
    QLabel      * sep[2];
    ArrowButton * up, * down;

protected slots:
    void increase();
    void decrease();
    void moveFocus();
};


class Q_EXPORT QDateEdit : public QDateTimeEditBase
{
    Q_OBJECT
    Q_PROPERTY( QDate date READ date WRITE setDate )
public:
    QDateEdit( QWidget * parent = 0, const char * name = 0 );
    void  setDate( const QDate & d );
    QDate date() const;

protected:
    void resizeEvent( QResizeEvent * );
};

class Q_EXPORT QTimeEdit : public QDateTimeEditBase
{
    Q_OBJECT
    Q_PROPERTY( QTime time READ time WRITE setTime )
public:
    QTimeEdit( QWidget * parent = 0, const char * name = 0 );
    void  setTime( const QTime & t );
    QTime time() const;

protected:
    void resizeEvent( QResizeEvent * );
};

#endif
#endif
