#include "colorchooser.h"

#include "qpainter.h"
#include "qlayout.h"
#include "qlabel.h"
#include "qwellarray.h"
#include "qpushbutton.h"

#include "qimage.h"
#include "qpixmap.h"

#include "qdrawutil.h"

#include <qapplication.h>


class QColorLuminancePicker : public QWidget
{
    Q_OBJECT
public:
    QColorLuminancePicker(QWidget* parent=0, const char* name=0);
    ~QColorLuminancePicker();

public slots:
    void setCol( int h, int s, int v );
    void setCol( int h, int s );

signals:
    void newVal( int v );
    
protected:
//    QSize sizeHint() const;
//    QSizePolicy sizePolicy() const;
    void paintEvent( QPaintEvent*);
    void mouseMoveEvent( QMouseEvent * );
    void mousePressEvent( QMouseEvent * );

private:
    int val;
    int hue;
    int sat;
    
    int valY() { return val ; }
    void setVal( int v );
};

QColorLuminancePicker::QColorLuminancePicker(QWidget* parent,
						  const char* name)
    :QWidget( parent, name )
{
    
}

QColorLuminancePicker::~QColorLuminancePicker()
{
    
}


void QColorLuminancePicker::mouseMoveEvent( QMouseEvent *m )
{
    setVal( 255-m->y()+2 );
}
void QColorLuminancePicker::mousePressEvent( QMouseEvent *m )
{
    setVal( 255-m->y()+2 );
}


void QColorLuminancePicker::setVal( int v ) 
{
    if ( val == v )
	return;
    val = QMAX( 0, QMIN(v,255)); 
    repaint( FALSE ); //#####
    emit newVal( val );
}

void QColorLuminancePicker::paintEvent( QPaintEvent * )
{
    QPainter p(this);
    int w = width() - 5;
    
    QRect r( 0, 0, w, height() );
    for ( int v = 0; v < 256; v++ ) {
	p.setPen( QColor( hue, sat, v, QColor::Hsv ) );
	p.drawLine( r.left()+2, 255-v + 2, r.right()-2,  255-v + 2 );
    }
    QColorGroup g = colorGroup();
    qDrawShadePanel( &p, r, g, TRUE );
    p.setPen( g.foreground() );
    p.setBrush( g.foreground() );
    QPointArray a;
    int y = 255-val + 2;
    a.setPoints( 3, w, y, w+5, y+5, w+5, y-5 );
    erase( w, 0, 5, height() );//###
    p.drawPolygon( a );
    
    
}

void QColorLuminancePicker::setCol( int h, int s , int v )
{
    val = v; 
    hue = h;
    sat = s;
    repaint( FALSE );//####
}

void QColorLuminancePicker::setCol( int h, int s )
{
    setCol( h, s, val );
}

class QColorPicker : public QFrame
{
    Q_OBJECT
public:
    QColorPicker(QWidget* parent=0, const char* name=0);
    ~QColorPicker();

public slots:
    void setCol( int h, int s );

signals:
    void newCol( int h, int s );
    
protected:
    QSize sizeHint() const;
    QSizePolicy sizePolicy() const;
    void drawContents(QPainter* p);
    void mouseMoveEvent( QMouseEvent * );
    void mousePressEvent( QMouseEvent * );

private:
    int hue;
    int sat;

    QPoint colPt() { return QPoint( 360-hue, 255-sat ); }
    int huePt( const QPoint &pt ) { return 360 - pt.x(); }
    int satPt( const QPoint &pt ) { return 255 - pt.y(); }
    void setCol( const QPoint &pt ) { setCol( huePt(pt), satPt(pt) ); }
    
    QPixmap *pix;
};

QColorPicker::QColorPicker(QWidget* parent=0, const char* name=0)
    : QFrame( parent, name )
{

    setCol( 150, 255 ); 
    
    debug( "QColorPicker::QColorPicker" );
    QImage img( 361, 256, 32 );
    for ( int hue = 0; hue <= 360; hue++ )
	    for ( int sat = 0; sat <= 255; sat++ )
		img.setPixel( 360-hue, 255-sat, qHsv(hue, sat, 255) );
    debug( "done creating QImage");
    pix = new QPixmap;
    pix->convertFromImage(img);
    debug( "done creating pixmap");
    
    setBackgroundMode( NoBackground );
}




QColorPicker::~QColorPicker()
{
    delete pix;
}

QSize QColorPicker::sizeHint() const
{
    return QSize( 360 + 2*frameWidth(), 256 + 2*frameWidth() );
}

QSizePolicy QColorPicker::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Fixed, QSizePolicy::Fixed );
}


void QColorPicker::setCol( int h, int s )
{
    int nhue = QMIN( QMAX(0,h), 360 ); 
    int nsat = QMIN( QMAX(0,s), 255);    
    if ( nhue == hue && nsat == sat )
	return;
    QRect r( colPt(), QSize(20,20) );
    hue = nhue; sat = nsat;
    r = r.unite( QRect( colPt(), QSize(20,20) ) );
    r.moveBy( contentsRect().x()-10, contentsRect().y()-10 );
    //    update( r );
    repaint( r, FALSE );
    emit newCol( hue, sat );
}


void QColorPicker::mouseMoveEvent( QMouseEvent *m )
{
    QPoint p = m->pos() - contentsRect().topLeft();
    setCol( p );
    
}

void QColorPicker::mousePressEvent( QMouseEvent *m )
{
    QPoint p = m->pos() - contentsRect().topLeft();
    setCol( p );    
}


void QColorPicker::drawContents(QPainter* p)
{
    QRect r = contentsRect();
    //QArray<QRect> rects =  p->clipRegion()->rects();

    p->drawPixmap( r.topLeft(), *pix );
    QPoint pt = colPt() + r.topLeft();
    p->setPen( QPen(black, 2) );
    p->drawLine( pt.x()-10, pt.y(), pt.x()+10, pt.y() );     
    p->drawLine( pt.x(), pt.y()-10, pt.x(), pt.y()+10 ); 
    
}


class QColorShower : public QLabel 
{
    Q_OBJECT
public:
    QColorShower( QWidget *parent, const char *name = 0 );

    QRgb currentColor() const { return curCol; }
    
public slots:    
    void setHsv( int h, int s, int v );
    void setHs( int h, int s );
    void setV( int v );
    //    void setRGB( );

private:
    int hue, sat, val;
    QRgb curCol;
    bool rgbOriginal;
};

QColorShower::QColorShower( QWidget *parent, const char *name = 0 )
    :QLabel( parent, name) 
{
    setHsv( 100, 100, 100 ); //###
}

void QColorShower::setHsv(  int h, int s, int v )
{
    rgbOriginal = FALSE;
    if ( h == hue && v == val && s == sat )
	return;
    hue = h; val = v; sat = s; //Range check###
    curCol = qHsv( hue, sat, val );
    QString str;
    str.sprintf( "Hue %d, Sat %d, Val %d \n %x"
		 , hue, sat, val, curCol );

    setBackgroundColor( QColor( curCol ) );
    setText( str );
    //emit
}

void QColorShower::setHs(  int h, int s )
{
    setHsv( h, s, val );
}


void QColorShower::setV(  int v )
{
    setHsv( hue, sat, v );
}




ColorChooser::ColorChooser(QWidget* parent, const char* name, bool modal) :
    QDialog(parent, name, modal)
{
    QHBoxLayout *topLay = new QHBoxLayout( this, 12, 6 );
    QVBoxLayout *leftLay = new QVBoxLayout;
    topLay->addLayout( leftLay );
    
    QLabel * lab = new QLabel( tr( "&Basic colors") , this );
    leftLay->addWidget( lab );
    standard = new QWellArray( this );
    standard->setDimension( 6, 8 );
    leftLay->addWidget( standard );

    
    leftLay->addStretch();

    lab = new QLabel( tr( "&Custom colors") , this );
    leftLay->addWidget( lab );
    custom = new QWellArray( this );
    custom->setDimension( 2, 8 );
    nCust = 0;
    
    
    int r, c;
    for ( c = 0; c < 8; c++ )
	for ( r = 0; r < 6; r++ )
	    standard->setCellBrush( r, c, QColor( r*255/6, c*255/8, (r+c)*15 ) ); //####

    for ( c = 0; c < 8; c++ )
	for ( r = 0; r < 2; r++ )
	    custom->setCellBrush( r, c, white );

    
    leftLay->addWidget( custom );
    
    QPushButton *custbut = new QPushButton( tr("&Define Custom Colors >>>"),
					    this );
    leftLay->addWidget( custbut );
    
    QHBoxLayout *buttons = new QHBoxLayout;
    leftLay->addLayout( buttons );

    QPushButton *ok, *cancel;
    ok = new QPushButton( "Ok", this );
    connect( ok, SIGNAL(clicked()), SLOT(accept()) );
    cancel = new QPushButton( "Cancel", this );
    connect( cancel, SIGNAL(clicked()), SLOT(reject()) );
    buttons->addWidget( ok );
    buttons->addWidget( cancel );
    buttons->addStretch();

    QVBoxLayout *rightLay = new QVBoxLayout;
    topLay->addLayout( rightLay );

    QHBoxLayout *pickLay = new QHBoxLayout;
    rightLay->addLayout( pickLay );
    
    QColorPicker *cp = new QColorPicker( this );
    cp->setFrameStyle( QFrame::Panel + QFrame::Sunken );
    pickLay->addWidget( cp );

    QColorLuminancePicker *lp = new QColorLuminancePicker( this );
    lp->setFixedWidth( 20 ); //##########
    pickLay->addWidget( lp );

    connect( cp, SIGNAL(newCol(int,int)), lp, SLOT(setCol(int,int)) );
    
    rightLay->addStretch();

    QColorShower *cs = new QColorShower( this );
    connect( cp, SIGNAL(newCol(int,int)), cs, SLOT(setHs(int,int)) );
    connect( lp, SIGNAL(newVal(int)), cs, SLOT(setV(int)) );
    rightLay->addWidget( cs );

    
    QPushButton *addCusBt = new QPushButton( tr("&Add To Custom Colors"),
					     this );
    rightLay->addWidget( addCusBt );
    connect( addCusBt, SIGNAL(clicked()), this, SLOT(addCustom()) );
}


void ColorChooser::addCustom()
{
    if ( nCust < 15 ) {
	custom->setCellBrush( nCust / 8, nCust % 8,  //###hardcode
			      QColor( cs->currentColor() ));
	nCust++;
	custom->repaint( FALSE ); //#############################
    }
}

main(int argc, char** argv)
{
    QApplication::setColorSpec( QApplication::ManyColor );
    QApplication app(argc, argv);
    QApplication::setFont( QFont("Helvetica") );

    ColorChooser m;
    m.show();

    QObject::connect(qApp, SIGNAL(lastWindowClosed()), qApp, SLOT(quit()));

    return app.exec();
}
#include "colorchooser.moc"
