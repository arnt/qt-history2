#include "qdatetimeedit.h"

#ifndef QT_NO_SQL

#include "qlayout.h"
#include "qtoolbutton.h"
#include "qpainter.h"
#include "qpushbutton.h"
#include "qframe.h"
#include "qlabel.h"

class NumEdit : public QLineEdit
{
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

class ArrowButton : public QToolButton
{
//    Q_OBJECT
public:
    typedef enum Direction { Up, Down };
    
    ArrowButton( Direction  dir = Up, QWidget * parent = 0, 
		 const char * name = 0) 
	: QToolButton( parent, name ), d( dir ){};
    
protected:
    void paintEvent( QPaintEvent * e )
    {
	QPainter p;
	QPointArray a;
	
	QToolButton::paintEvent( e );
	if( p.begin( this ) ){
	    switch( d ){
		case Up:
		    a.setPoints( 4, width()/2,2, width()-4,height()-4,
				 3,height()-4, width()/2,2 );
		    break;
		case Down:
		    a.setPoints( 4, 3,2, width()-4,2,
				 width()/2,height()-4, 3,2 );
		    break;
		default: 
		    break;		    
	    }
	    p.setPen( palette().active().buttonText() );
	    p.setBrush( QBrush( palette().active().buttonText() ) );
	    p.drawPolygon( a );
	    p.end();
	}
    }
    
    Direction d;
};

QDateTimeEditBase::QDateTimeEditBase( QWidget * parent, const char * name )
    : QWidget( parent, name )
{
    e[0] = new NumEdit( this );
    e[1] = new NumEdit( this );
    e[2] = new NumEdit( this );

    connect( e[0], SIGNAL( returnPressed() ), SLOT( moveFocus() ) );
    connect( e[1], SIGNAL( returnPressed() ), SLOT( moveFocus() ) );
    connect( e[2], SIGNAL( returnPressed() ), SLOT( moveFocus() ) );

    sep[0] = new QLabel( this );
    sep[1] = new QLabel( this );

    sep[0]->setBackgroundColor( e[0]->backgroundColor() );
    sep[1]->setBackgroundColor( e[0]->backgroundColor() );
    setBackgroundColor( e[0]->backgroundColor() );

    up = new ArrowButton( ArrowButton::Up, this );
//    up->setText( "." );
    up->setAutoRepeat( TRUE );
    down = new ArrowButton( ArrowButton::Down, this );
  //  down->setText( "." );
    down->setAutoRepeat( TRUE );

    connect( up, SIGNAL( clicked() ), SLOT( increase() ) );
    connect( down, SIGNAL( clicked() ), SLOT( decrease() ) );
    setFocusProxy( e[0] );
    e[0]->installEventFilter( this );
    e[1]->installEventFilter( this );
    e[2]->installEventFilter( this );
}

void QDateTimeEditBase::increase()
{
    QWidget * f = focusWidget();

    if( f->inherits("NumEdit") ){
	NumEdit * e = (NumEdit *) f;
	QIntValidator * v = (QIntValidator *) e->validator();

	int n = e->text().toInt();
	n++;

	if( n > v->top() )
	    n = v->top();
	else if( n < v->bottom() )
	    n = v->bottom();

	e->setText( QString().setNum( n ) );
    }
}

void QDateTimeEditBase::decrease()
{
    QWidget * f = focusWidget();

    if( f->inherits("NumEdit") ){
	NumEdit * e = (NumEdit *) f;
	QIntValidator * v = (QIntValidator *) e->validator();

	int n = e->text().toInt();
	n--;

	if( n > v->top() )
	    n = v->top();
	else if( n < v->bottom() )
	    n = v->bottom();

	e->setText( QString().setNum( n ) );
    }
}

void QDateTimeEditBase::moveFocus()
{
    focusNextPrevChild( TRUE );
}

bool QDateTimeEditBase::eventFilter( QObject *, QEvent * ev )
{
    if( ev->type() == QEvent::KeyPress ){
	QKeyEvent * k = (QKeyEvent *) ev;
	if( k->key() == Qt::Key_Tab ){
	    qDebug("foo");
	    QWidget * focus = focusWidget();
	    int i = 0;
	    if( focus == e[0] ){
		i = 0;
	    } else if( focus == e[1] ){
		i = 1;
	    } else if( focus == e[2] ){
		i = 2;
	    }
	    QString s = e[i]->text();
	    int pos = 0;
	    if( e[i]->validator()->validate( s, pos ) !=
		QValidator::Acceptable )
	    {
		e[i]->setSelection( 0, e[i]->text().length() );
		return TRUE;
	    }
	}
    }
    return FALSE;
}

/*!

  A small convenient editor for editing dates.
 */
QDateEdit::QDateEdit( QWidget * parent, const char * name )
    : QDateTimeEditBase( parent, name )
{
    e[0]->setRange( 1970, 2999 );
    e[1]->setRange( 1, 12 );
    e[2]->setRange( 1, 31 );
    sep[0]->setText( "-" );
    sep[1]->setText( "-" );
}

void QDateEdit::setDate( const QDate & d )
{
    e[0]->setText( QString().setNum( d.year() ) );
    e[1]->setText( QString().setNum( d.month() ) );
    e[2]->setText( QString().setNum( d.day() ) );

    e[0]->selectAll();
    e[0]->setFocus();
}

QDate QDateEdit::date() const
{
    return QDate( e[0]->text().toInt(), e[1]->text().toInt(),
		  e[2]->text().toInt() );
}

void QDateEdit::resizeEvent( QResizeEvent * )
{
    int mSize = 6;
    int numSize = (width() - 15) / 10;
    int h = height();
    
    e[0]->resize( numSize*4, h );

    e[1]->resize( numSize*2, h );
    e[1]->move( e[0]->x() + e[0]->width() + mSize, 0 );

    e[2]->resize( numSize*2, h );
    e[2]->move( e[1]->x() + e[1]->width() + mSize, 0 );

    sep[0]->resize( mSize, h );
    sep[1]->resize( mSize, h );
    sep[0]->move( e[0]->x() + e[0]->width() + 1, -2 );
    sep[1]->move( e[1]->x() + e[1]->width() + 1, -2 );

    up->resize( 15, h/2 );
    down->resize( 15, h/2 );

    up->move( width() - 15, 0 );
    down->move( width() - 15, h/2 );
}

/*!

  A small convenient editor for editing time.
 */
QTimeEdit::QTimeEdit( QWidget * parent, const char * name )
    : QDateTimeEditBase( parent, name )
{
    e[0]->setRange( 0, 23 );
    e[1]->setRange( 0, 59 );
    e[2]->setRange( 0, 59 );
    sep[0]->setText( ":" );
    sep[1]->setText( ":" );
}

void QTimeEdit::setTime( const QTime & t )
{
    e[0]->setText( QString().setNum( t.hour() ) );
    e[1]->setText( QString().setNum( t.minute() ) );
    e[2]->setText( QString().setNum( t.second() ) );
    
    e[0]->selectAll();
    e[0]->setFocus();
}

QTime QTimeEdit::time() const
{
    return QTime( e[0]->text().toInt(), e[1]->text().toInt(),
		  e[2]->text().toInt() );
}

void QTimeEdit::resizeEvent( QResizeEvent * )
{
    int mSize = 6;
    int numSize = (width() - 15) / 8;
    int h = height();
        
    e[0]->resize( numSize*2, h );

    e[1]->resize( numSize*2, h );
    e[1]->move( e[0]->x() + e[0]->width() + mSize, 0 );

    e[2]->resize( numSize*2, h);
    e[2]->move( e[1]->x() + e[1]->width() + mSize, 0 );

    sep[0]->resize( mSize, h );
    sep[1]->resize( mSize, h );
    sep[0]->move( e[0]->x() + e[0]->width() + 1, -2 );
    sep[1]->move( e[1]->x() + e[1]->width() + 1, -2 );

    up->resize( 15, h/2 );
    down->resize( 15, h/2 );

    up->move( width() - 15, 0 );
    down->move( width() - 15, h/2 );
}

#endif
