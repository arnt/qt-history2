#include "qdatetimeedit.h"

#ifndef QT_NO_SQL

#include "qlayout.h"
#include "qtoolbutton.h"
#include "qpainter.h"
#include "qpushbutton.h"
#include "qpixmapcache.h"
#include "qbitmap.h"
#include "qframe.h"
#include "qlabel.h"
#include "qapplication.h"

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


/*!
  
  Base class for the QTimeEdit and QDateEdit widgets.
 */
QDateTimeEditBase::QDateTimeEditBase( QWidget * parent, const char * name )
    : QFrame( parent, name )
{
    if( style() == WindowsStyle )
	setFrameStyle( WinPanel | Sunken );
    else
	setFrameStyle( Panel | Sunken );
    setLineWidth( 2 );
    
    ed[0] = new NumEdit( this );
    ed[1] = new NumEdit( this );
    ed[2] = new NumEdit( this );

    connect( ed[0], SIGNAL( returnPressed() ), SLOT( moveFocus() ) );
    connect( ed[1], SIGNAL( returnPressed() ), SLOT( moveFocus() ) );
    connect( ed[2], SIGNAL( returnPressed() ), SLOT( moveFocus() ) );

    sep[0] = new QLabel( this );
    sep[1] = new QLabel( this );

    sep[0]->setBackgroundColor( ed[0]->backgroundColor() );
    sep[1]->setBackgroundColor( ed[0]->backgroundColor() );
    setBackgroundColor( ed[0]->backgroundColor() );

    up   = new QPushButton( this );
    down = new QPushButton( this );
    up->setAutoRepeat( TRUE );
    down->setAutoRepeat( TRUE );

    connect( up, SIGNAL( clicked() ), SLOT( stepUp() ) );
    connect( down, SIGNAL( clicked() ), SLOT( stepDown() ) );
    
    ed[0]->installEventFilter( this );
    ed[1]->installEventFilter( this );
    ed[2]->installEventFilter( this );

    setFocusProxy( ed[0] );
}

/*!
  
  Draw the arrow buttons.
 */
void QDateTimeEditBase::updateArrows()
{
    QString key( QString::fromLatin1( "$qt$qspinbox$" ) );
    bool pmSym = false;
    key += QString::fromLatin1( pmSym ? "+-" : "^v" );
    key += QString::number( down->height() );
    QString upKey = key + QString::fromLatin1( "$up" );
    QString dnKey = key + QString::fromLatin1( "$down" );
    QBitmap upBm;
    QBitmap dnBm;

    bool found = QPixmapCache::find( dnKey, dnBm )
		 && QPixmapCache::find( upKey, upBm );

    if ( !found ) {
	QPainter p;
	int w = down->width()-4;
	if ( w < 3 )
	    return;
	else if ( !(w & 1) )
	    w--;
	w -= ( w / 7 ) * 2;	// Empty border
	int h = w/2 + 2;        // Must have empty row at foot of arrow
	dnBm.resize( w, h );
	p.begin( &dnBm );
	p.eraseRect( 0, 0, w, h );
	QPointArray a;
	a.setPoints( 3,  0, 1,  w-1, 1,  h-2, h-1 );
	p.setBrush( color1 );
	p.drawPolygon( a );
	p.end();
#ifndef QT_NO_TRANSFORMATIONS
	QWMatrix wm;
	wm.scale( 1, -1 );
	upBm = dnBm.xForm( wm );
#else
	upBm.resize( w, h );
	p.begin( &upBm );
	p.eraseRect( 0, 0, w, h );
	a.setPoints( 3,  0, h-2,  w-1, h-2,  h-2, 0 );
	p.setBrush( color1 );
	p.drawPolygon( a );
	p.end();
#endif
	QPixmapCache::insert( dnKey, dnBm );
	QPixmapCache::insert( upKey, upBm );
    }
    down->setPixmap( dnBm );
    up->setPixmap( upBm );
}

/*!
  
  Increases the current value one step. This slot is called when the
  up button is clicked.
 */
void QDateTimeEditBase::stepUp()
{
    QWidget * f = focusWidget();

    if( f->inherits("QLineEdit") ){
	NumEdit * e = (NumEdit *) f;
	QIntValidator * v = (QIntValidator *) e->validator();

	int n = e->text().toInt();
	n++;

	if( n > v->top() )
	    n = v->top();
	else if( n < v->bottom() )
	    n = v->bottom();

	e->setText( QString::number( n ) );
    }
}

/*!
  
  Decreases the current value one step. This slot is called when the
  down button is clicked.
 */
void QDateTimeEditBase::stepDown()
{
    QWidget * f = focusWidget();

    if( f->inherits("QLineEdit") ){
	NumEdit * e = (NumEdit *) f;
	QIntValidator * v = (QIntValidator *) e->validator();

	int n = e->text().toInt();
	n--;

	if( n > v->top() )
	    n = v->top();
	else if( n < v->bottom() )
	    n = v->bottom();

	e->setText( QString::number( n ) );
    }
}

void QDateTimeEditBase::moveFocus()
{
    focusNextPrevChild( TRUE );
}

/*!
  
  \reimp
 */
bool QDateTimeEditBase::eventFilter( QObject *, QEvent * ev )
{
    if( ev->type() == QEvent::FocusOut ){
	for(int i = 0; i < 3; i++){
	    QString s = ed[i]->text(); 
	    int pos = 0;

	    if( ed[i]->validator()->validate( s, pos ) !=  
		QValidator::Acceptable )
	    {
		ed[i]->setText( lastValid[i] );
	    } else {
		lastValid[i] = ed[i]->text();
	    }
	}	
    }    
    return FALSE;
}

/*!
  
  Layout the different widgets.
 */
void QDateTimeEditBase::layoutWidgets( int numDigits )
{
    int mSize = 6;
    int h     = height() - frameWidth()*2;
    int numSize = (width() - 15) / numDigits;
    int offset  = frameWidth();
    
    if( numDigits != 8 ){
	ed[0]->resize( numSize*4, h );
    } else {
	ed[0]->resize( numSize*2, h );
    }
    ed[0]->move( offset, offset );
    
    ed[1]->resize( numSize*2, h );
    ed[1]->move( ed[0]->x() + ed[0]->width() + mSize, offset );

    ed[2]->resize( numSize*2, h );
    ed[2]->move( ed[1]->x() + ed[1]->width() + mSize, offset );

    sep[0]->resize( mSize, h - 2);
    sep[1]->resize( mSize, h - 2);
    sep[0]->move( ed[0]->x() + ed[0]->width() + 1, offset );
    sep[1]->move( ed[1]->x() + ed[1]->width() + 1, offset );
    
    QSize bs; 
    if ( style() == WindowsStyle )
	bs.setHeight( height()/2 - frameWidth() );
    else
	bs.setHeight( height()/2 );
    if ( bs.height() < 8 )
	bs.setHeight( 8 );
    bs.setWidth( bs.height() * 8 / 5 ); // 1.6 - approximate golden mean

    int y = style() == WindowsStyle ? frameWidth() : 0;
    int x, lx, rx;
    if ( QApplication::reverseLayout() ) {
	x = y;
	lx = x + bs.width() + frameWidth();
	rx = width() - frameWidth();
    } else {
	x = width() - y - bs.width();
	lx = frameWidth();
	rx = x - frameWidth();
    }

    if ( style() == WindowsStyle )
	setFrameRect( QRect( 0, 0, 0, 0 ) );
    else
	setFrameRect( QRect( lx - frameWidth(), 0, width() - bs.width(),
			     height() ) );

    if ( up->size() != bs || down->size() != bs ) {
	up->resize( bs );
	down->resize( bs );
	updateArrows();
    }

    up->move( x, y );
    down->move( x, height() - y - up->height() );
}



/*!
  \class QDateEdit qdatetimeedit.h
  \brief The QDateEdit class provides a spin-like box to edit dates
  
  The QDateEdit class provides a spin-like box to edit dates.
  
  QDateEdit allows the user to edit the date by either using the
  keyboard, or by using the arrow buttons to increase/decrease date
  values.  The Tab key can be used to move from field to field within
  the QDateEdit box.
  
  If illegal values are entered, these will be reverted to the last
  known legal value. I.e if you enter 5000 for the day value, and
  it was 12 before you stated editing, the value will be reverted to
  12.
  
 */
QDateEdit::QDateEdit( QWidget * parent, const char * name )
    : QDateTimeEditBase( parent, name )
{
    ed[0]->setRange( 1753, 3000 );
    ed[1]->setRange( 1, 12 );
    ed[2]->setRange( 1, 31 );
    sep[0]->setText( "-" );
    sep[1]->setText( "-" );
}

/*!
  
  Set the date in this QDateEdit.
 */
void QDateEdit::setDate( const QDate & d )
{
    QIntValidator * v[3];
    int yy = d.year();
    int mm = d.month();
    int dd = d.day();
    
    v[0] = (QIntValidator *) ed[0]->validator();
    v[1] = (QIntValidator *) ed[1]->validator();
    v[2] = (QIntValidator *) ed[2]->validator();

    if( (yy > v[0]->top()) || (yy < v[0]->bottom()) || 
	(mm > v[1]->top()) || (mm < v[1]->bottom()) || 
	(dd > v[2]->top()) || (dd < v[2]->bottom()) )
    {
	// Date out of range - leave it blank
	ed[0]->setText( "" );
	ed[1]->setText( "" );
	ed[2]->setText( "" );
    } else {
	ed[0]->setText( QString::number( yy ) );
	ed[1]->setText( QString::number( mm ) );
	ed[2]->setText( QString::number( dd ) );
    }

    ed[0]->setFocus();
    ed[0]->selectAll();
}

/*!
  
  Returns the date in this QDateEdit.
 */
QDate QDateEdit::date() const
{
    ((QDateEdit *) this)->fixup(); // Fix invalid dates
    
    return QDate( ed[0]->text().toInt(), ed[1]->text().toInt(),
		  ed[2]->text().toInt() );
}

/*!  
  
  Post-process the edited date. This will guarantee that a date is
  valid.
*/
void QDateEdit::fixup()
{
    int yy, mm, dd;
    QIntValidator * v[3];
    v[0] = (QIntValidator *) ed[0]->validator();
    v[1] = (QIntValidator *) ed[1]->validator();
    v[2] = (QIntValidator *) ed[2]->validator();

    yy = ed[0]->text().toInt();
    mm = ed[1]->text().toInt();
    dd = ed[2]->text().toInt();
    
    if( !QDate::isValid( yy, mm, dd) ){
	if( !QDate::isValid( yy, 1, 1 ) )
	    if( yy > v[0]->top() ) yy = v[0]->top();
	    else if( yy < v[0]->bottom() ) yy = v[0]->bottom();
	if( !QDate::isValid( yy, mm, 1 ) )
	    if( mm > v[1]->top() ) mm = v[1]->top();
	    else if( mm < v[1]->bottom() ) mm = v[1]->bottom();
	if( dd > v[2]->top() ) dd = v[2]->top();
	else if( dd < v[2]->bottom() ) dd = v[2]->bottom();
	
	while( !QDate::isValid( yy, mm, dd ) ){
	    dd--;
	}
	ed[0]->setText( QString::number( yy ) );
	ed[1]->setText( QString::number( mm ) );
	ed[2]->setText( QString::number( dd ) );
    }
}

/*!  
  
  Handle resize events.
*/
void QDateEdit::resizeEvent( QResizeEvent * )
{
    layoutWidgets( 10 );
}

/*!

  \class QTimeEdit qdatetimeedit.h
  \brief The QTimeEdit class provides a spin-like box to edit a given
  time
  
  The QTimeEdit class provides a spin-like box to edit a given time.
  
  QTimeEdit allows the user to edit the time by either using the
  keyboard, or by using the arrow buttons to increase/decrease date
  values.  The Tab key can be used to move from field to field within
  the QTimeEdit box.
  
  If illegal values are entered, these will be reverted to the last
  known legal value. I.e if you enter 5000 for the hour value, and
  it was 12 before you stated editing, the value will be reverted to
  12.
  */
QTimeEdit::QTimeEdit( QWidget * parent, const char * name )
    : QDateTimeEditBase( parent, name )
{
    ed[0]->setRange( 0, 23 );
    ed[1]->setRange( 0, 59 );
    ed[2]->setRange( 0, 59 );
    sep[0]->setText( ":" );
    sep[1]->setText( ":" );
}

/*!
  
  Set the time in this QTimeEdit.
 */
void QTimeEdit::setTime( const QTime & t )
{
    ed[0]->setText( QString::number( t.hour() ) );
    ed[1]->setText( QString::number( t.minute() ) );
    ed[2]->setText( QString::number( t.second() ) );
    
    ed[0]->setFocus();
    ed[0]->selectAll();
}

/*!
  
  Returns the time in this QTimeEdit.
 */
QTime QTimeEdit::time() const
{
    return QTime( ed[0]->text().toInt(), ed[1]->text().toInt(),
		  ed[2]->text().toInt() );
}

/*!
  
  Post-process the edited time.
 */
void QTimeEdit::fixup()
{
}

void QTimeEdit::resizeEvent( QResizeEvent * )
{
    layoutWidgets( 8 );
}
#endif
