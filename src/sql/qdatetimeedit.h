#include <qwidget.h>
#include <qvalidator.h>
#include <qstring.h>
#include <qdatetime.h>
#include <qlineedit.h>

class NumEdit : public QLineEdit
{
    Q_OBJECT
public:
    NumEdit( QWidget * parent, const char * name = 0 )
	: QLineEdit( parent, name )
    {
	setFrame( FALSE );
	setAlignment( AlignRight );
    }
    
    void setRange( int min, int max )
    {
	QIntValidator * v = new QIntValidator( this );
	v->setRange( min, max );
	setValidator( v );
    }
};

class QLabel;
class QToolButton;

class QDateTimeEditBase : public QWidget
{
    Q_OBJECT
public:
    QDateTimeEditBase( QWidget * parent = 0, const char * name = 0 );    
    
protected:
    bool eventFilter( QObject *, QEvent * );
    
    NumEdit     * e[3];
    QLabel      * sep[2];
    QToolButton * up, * down;

protected slots:
    void increase();
    void decrease();
    void moveFocus();
};


class QDateEdit : public QDateTimeEditBase 
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

class QTimeEdit : public QDateTimeEditBase 
{
    Q_PROPERTY( QTime time READ time WRITE setTime )
public:
    QTimeEdit( QWidget * parent = 0, const char * name = 0 );
    void  setTime( const QTime & t );
    QTime time() const;

protected:
    void resizeEvent( QResizeEvent * );
};
