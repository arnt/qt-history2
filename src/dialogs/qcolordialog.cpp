/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qcolordialog.cpp#2 $
**
** Implementation of QColorDialog class
**
** Created : 990222
**
** Copyright (C) 1999 Troll Tech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Troll Tech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** Licensees with valid Qt Professional Edition licenses may distribute and
** use this file in accordance with the Qt Professional Edition License
** provided at sale or upon request.
**
** See http://www.troll.no/pricing.html or email sales@troll.no for
** information about the Professional Edition licensing, or see
** http://www.troll.no/qpl/ for QPL licensing information.
**
*****************************************************************************/

#include "qcolordialog.h"

#include "qpainter.h"
#include "qlayout.h"
#include "qlabel.h"
#include "qwellarray.h"
#include "qpushbutton.h"
#include "qlineedit.h"
#include "qimage.h"
#include "qpixmap.h"
#include "qdrawutil.h"

static bool initrgb = FALSE;
static QRgb stdrgb[6*8];
static QRgb cusrgb[2*8];


static inline void rgb2hsv( QRgb rgb, int&h, int&s, int&v )
{
    QColor c;
    c.setRgb( rgb );
    c.getHsv(h,s,v);
}



class QColorWell : public QWellArray
{
public:
    QColorWell( QWidget *parent, int r, int c, QRgb *vals )
	:QWellArray( parent ), values( vals ) { setDimension(r,c); }
protected:
    void drawContents( QPainter *, int row, int col, const QRect& );
private:
    QRgb *values;
};

void QColorWell::drawContents( QPainter *p, int row, int col, const QRect &r )
{
    int i = row + col*numRows();
    p->fillRect( r, QColor( values[i] ) );
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

    QPoint colPt();
    int huePt( const QPoint &pt );
    int satPt( const QPoint &pt );
    void setCol( const QPoint &pt );

    QPixmap *pix;
};

static const int pWidth = 200;
static const int pHeight = 200;

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
    void newHsv( int h, int s, int v );

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

    int y2val( int y );
    int val2y( int val );
    void setVal( int v );
};

int QColorLuminancePicker::y2val( int y )
{
    int h = height() - 4;
    return 255 - (y - 2)*255/h;
}

int QColorLuminancePicker::val2y( int v )
{
    int h = height() - 4;
    return 2 + (255-v)*h/255;
}

QColorLuminancePicker::QColorLuminancePicker(QWidget* parent,
						  const char* name)
    :QWidget( parent, name )
{
    hue = 100; val = 100; sat = 100;
}

QColorLuminancePicker::~QColorLuminancePicker()
{

}

void QColorLuminancePicker::mouseMoveEvent( QMouseEvent *m )
{
    setVal( y2val(m->y()) );
}
void QColorLuminancePicker::mousePressEvent( QMouseEvent *m )
{
    setVal( y2val(m->y()) );
}

void QColorLuminancePicker::setVal( int v )
{
    if ( val == v )
	return;
    val = QMAX( 0, QMIN(v,255));
    repaint( FALSE ); //###
    emit newHsv( hue, sat, val );
}

//receives from a hue,sat chooser and relays.
void QColorLuminancePicker::setCol( int h, int s )
{
    setCol( h, s, val );
    emit newHsv( h, s, val );
}

void QColorLuminancePicker::paintEvent( QPaintEvent * )
{
    QPainter p(this);
    int w = width() - 5;

    QRect r( 0, 0, w, height() );
    for ( int y = r.top() + 2; y < r.bottom() - 2; y++ ) {
	p.setPen( QColor( hue, sat, y2val(y), QColor::Hsv ) );
	p.drawLine( r.left()+2, y, r.right()-2,  y );
    }
    QColorGroup g = colorGroup();
    qDrawShadePanel( &p, r, g, TRUE );
    p.setPen( g.foreground() );
    p.setBrush( g.foreground() );
    QPointArray a;
    int y = val2y(val);
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

QPoint QColorPicker::colPt()
{ return QPoint( (360-hue)*pWidth/360, (255-sat)*pHeight/255 ); }
int QColorPicker::huePt( const QPoint &pt )
{ return 360 - pt.x()*360/pHeight; }
int QColorPicker::satPt( const QPoint &pt )
{ return 255 - pt.y()*255/pWidth ; }
void QColorPicker::setCol( const QPoint &pt )
{ setCol( huePt(pt), satPt(pt) ); }

QColorPicker::QColorPicker(QWidget* parent=0, const char* name=0)
    : QFrame( parent, name )
{
    setCol( 150, 255 );

    QImage img( pHeight, pWidth, 32 );
    int x,y;
    for ( y = 0; y < pHeight; y++ )
    for ( x = 0; x < pWidth; x++ ) {
	    QPoint p( x, y );
	    img.setPixel( x, y,
			  QColor(huePt(p), satPt(p), 200, QColor::Hsv).rgb()
			  );
	}
    pix = new QPixmap;
    pix->convertFromImage(img);
    setBackgroundMode( NoBackground );
}

QColorPicker::~QColorPicker()
{
    delete pix;
}

QSize QColorPicker::sizeHint() const
{
    return QSize( pHeight + 2*frameWidth(), pWidth + 2*frameWidth() );
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
}

void QColorPicker::mouseMoveEvent( QMouseEvent *m )
{
    QPoint p = m->pos() - contentsRect().topLeft();
    setCol( p );
    emit newCol( hue, sat );
}

void QColorPicker::mousePressEvent( QMouseEvent *m )
{
    QPoint p = m->pos() - contentsRect().topLeft();
    setCol( p );
    emit newCol( hue, sat );
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

class QColNumLineEdit;
class QColorShowLabel;

class QColorShower : public QWidget
{
    Q_OBJECT
public:
    QColorShower( QWidget *parent, const char *name = 0 );

    //things that don't emit signals
    void setHsv( int h, int s, int v );
    void setRgb( QRgb rgb );
    //    void setRGB( );
    QRgb currentColor() const { return curCol; }
signals:
    void newCol( QRgb rgb );
private slots:
    void rgbEd();
    void hsvEd();
private:
    void showCurrentColor();
    int hue, sat, val;
    QRgb curCol;
    QColNumLineEdit *hEd;
    QColNumLineEdit *sEd;
    QColNumLineEdit *vEd;
    QColNumLineEdit *rEd;
    QColNumLineEdit *gEd;
    QColNumLineEdit *bEd;
    QColorShowLabel *lab;
    bool rgbOriginal;
};

class QColNumLineEdit : public QLineEdit
{
public:
    QColNumLineEdit( QWidget *parent, const char* name = 0 )
	: QLineEdit( parent, name ) { setMaxLength( 3 );} //###validator
    QSize sizeHint() const {
	return QSize( 30, //#####
		     QLineEdit::sizeHint().height() ); }
    void setNum( int i ) {
	QString s;
	s.setNum(i);
	blockSignals(TRUE);
	setText( s );
	blockSignals(FALSE);
    }
    int val() const { return text().toInt(); }
};

class QColorShowLabel : public QFrame
{
public:
    QColorShowLabel( QWidget *parent ) :QFrame( parent ) {
	setFrameStyle( QFrame::Panel|QFrame::Sunken );
	setBackgroundMode( PaletteBackground );
    }
    void setColor( QColor c ) { col = c; }
protected:
    void drawContents( QPainter *p );
private:
    QColor col;
};

void QColorShowLabel::drawContents( QPainter *p )
{
    p->fillRect( contentsRect(), col );
}

QColorShower::QColorShower( QWidget *parent, const char *name = 0 )
    :QWidget( parent, name)
{
    QGridLayout *gl = new QGridLayout( this, 1, 1, 6 );
    lab = new QColorShowLabel( this );
    lab->setMinimumWidth( 60 ); //###
    gl->addMultiCellWidget(lab, 0,-1,0,0);

    hEd = new QColNumLineEdit( this );
    QLabel *l = new QLabel( hEd, tr("Hu&e:"), this );
    l->setAlignment( AlignRight );
    gl->addWidget( l, 0, 1 );
    gl->addWidget( hEd, 0, 2 );

    sEd = new QColNumLineEdit( this );
    l = new QLabel( sEd, tr("&Sat:"), this );
    l->setAlignment( AlignRight );
    gl->addWidget( l, 1, 1 );
    gl->addWidget( sEd, 1, 2 );

    vEd = new QColNumLineEdit( this );
    l = new QLabel( vEd, tr("&Val:"), this );
    l->setAlignment( AlignRight );
    gl->addWidget( l, 2, 1 );
    gl->addWidget( vEd, 2, 2 );

    rEd = new QColNumLineEdit( this );
    l = new QLabel( hEd, tr("&Red:"), this );
    l->setAlignment( AlignRight );
    gl->addWidget( l, 0, 3 );
    gl->addWidget( rEd, 0, 4 );

    gEd = new QColNumLineEdit( this );
    l = new QLabel( sEd, tr("&Green:"), this );
    l->setAlignment( AlignRight );
    gl->addWidget( l, 1, 3 );
    gl->addWidget( gEd, 1, 4 );

    bEd = new QColNumLineEdit( this );
    l = new QLabel( bEd, tr("Bl&ue:"), this );
    l->setAlignment( AlignRight );
    gl->addWidget( l, 2, 3 );
    gl->addWidget( bEd, 2, 4 );

    connect( hEd, SIGNAL(textChanged(const QString&)), this, SLOT(hsvEd()) );
    connect( sEd, SIGNAL(textChanged(const QString&)), this, SLOT(hsvEd()) );
    connect( vEd, SIGNAL(textChanged(const QString&)), this, SLOT(hsvEd()) );

    connect( rEd, SIGNAL(textChanged(const QString&)), this, SLOT(rgbEd()) );
    connect( gEd, SIGNAL(textChanged(const QString&)), this, SLOT(rgbEd()) );
    connect( bEd, SIGNAL(textChanged(const QString&)), this, SLOT(rgbEd()) );
}

void QColorShower::showCurrentColor()
{
    lab->setColor( currentColor() );
    lab->repaint(FALSE); //###
}

void QColorShower::rgbEd()
{
    rgbOriginal = TRUE;
    curCol = qRgb( rEd->val(), gEd->val(), bEd->val() );
    rgb2hsv(currentColor(), hue, sat, val );

    hEd->setNum( hue );
    sEd->setNum( sat );
    vEd->setNum( val );

    showCurrentColor();
    emit newCol( currentColor() );
}

void QColorShower::hsvEd()
{
    rgbOriginal = FALSE;
    hue = hEd->val();
    sat = sEd->val();
    val = vEd->val();

    curCol = QColor( hue, sat, val, QColor::Hsv ).rgb();

    rEd->setNum( qRed(currentColor()) );
    gEd->setNum( qGreen(currentColor()) );
    bEd->setNum( qBlue(currentColor()) );

    showCurrentColor();
    emit newCol( currentColor() );
}

void QColorShower::setRgb( QRgb rgb )
{
    rgbOriginal = TRUE;
    if ( currentColor() == rgb )
	return;
    curCol = rgb;

    rgb2hsv(currentColor(), hue, sat, val );

    hEd->setNum( hue );
    sEd->setNum( sat );
    vEd->setNum( val );

    rEd->setNum( qRed(currentColor()) );
    gEd->setNum( qGreen(currentColor()) );
    bEd->setNum( qBlue(currentColor()) );

    showCurrentColor();
}

void QColorShower::setHsv(  int h, int s, int v )
{
    rgbOriginal = FALSE;
    if ( h == hue && v == val && s == sat )
	return;
    hue = h; val = v; sat = s; //Range check###
    curCol = QColor( hue, sat, val, QColor::Hsv ).rgb();

    hEd->setNum( hue );
    sEd->setNum( sat );
    vEd->setNum( val );

    rEd->setNum( qRed(currentColor()) );
    gEd->setNum( qGreen(currentColor()) );
    bEd->setNum( qBlue(currentColor()) );


    showCurrentColor();
}

class QColorDialogPrivate : public QObject
{
Q_OBJECT
public:
    QColorDialogPrivate( QColorDialog *p );
    QRgb currentColor() const { return cs->currentColor(); }

private slots:
    void addCustom();

    void newHsv( int h, int s, int v );
    void newColorTypedIn( QRgb rgb );
    void newCustom( int, int );
    void newStandard( int, int );
private:
    void newRgb( QRgb rgb );
    QColorPicker *cp;
    QColorLuminancePicker *lp;
    QWellArray *custom;
    QWellArray *standard;
    QColorShower *cs;
    int nCust;

};

//sets all widgets to display h,s,v
void QColorDialogPrivate::newHsv( int h, int s, int v )
{
    cs->setHsv( h, s, v );
    cp->setCol( h, s );
    lp->setCol( h, s, v );
}

//sets all widgets to display rgb
void QColorDialogPrivate::newRgb( QRgb rgb )
{
    cs->setRgb( rgb );
    newColorTypedIn( rgb );
}

//sets all widgets exept cs to display rgb
void QColorDialogPrivate::newColorTypedIn( QRgb rgb )
{
    int h, s, v;
    rgb2hsv(rgb, h, s, v );
    cp->setCol( h, s );
    lp->setCol( h, s, v);
}

void QColorDialogPrivate::newCustom( int r, int c )
{
    newRgb( cusrgb[r+c*2] ); //###
    standard->setSelected(-1,-1);
}

void QColorDialogPrivate::newStandard( int r, int c )
{
    newRgb( stdrgb[r+c*6] ); //###
    custom->setSelected(-1,-1);
}

QColorDialogPrivate::QColorDialogPrivate( QColorDialog *dialog )
{
    QHBoxLayout *topLay = new QHBoxLayout( dialog, 12, 6 );
    QVBoxLayout *leftLay = new QVBoxLayout;
    topLay->addLayout( leftLay );

    if ( !initrgb ) {
	initrgb = TRUE;
	int i = 0;
	for ( int g = 0; g < 4; g++ )
	    for ( int r = 0;  r < 4; r++ )
		for ( int b = 0; b < 3; b++ )
		    stdrgb[i++] = qRgb( r*255/3, g*255/3, b*255/2 );

	for ( i = 0; i < 2*8; i++ )
	    cusrgb[i] = qRgb(0xff,0xff,0xff);
    }


    standard = new QColorWell( dialog, 6, 8, stdrgb );
    standard->setCellSize( 28, 24 );
    QLabel * lab = new QLabel( standard, tr( "&Basic colors") , dialog );
    connect( standard, SIGNAL(selected(int,int)), SLOT(newStandard(int,int)));
    leftLay->addWidget( lab );
    leftLay->addWidget( standard );


    leftLay->addStretch();

    custom = new QColorWell( dialog, 2, 8, cusrgb );
    custom->setCellSize( 28, 24 );
    nCust = 0;

    connect( custom, SIGNAL(selected(int,int)), SLOT(newCustom(int,int)));
    lab = new QLabel( custom, tr( "&Custom colors") , dialog );
    leftLay->addWidget( lab );
    leftLay->addWidget( custom );

    QPushButton *custbut = new QPushButton( tr("&Define Custom Colors >>"),
					    dialog );
    custbut->setEnabled( FALSE );
    leftLay->addWidget( custbut );

    QHBoxLayout *buttons = new QHBoxLayout;
    leftLay->addLayout( buttons );

    QPushButton *ok, *cancel;
    ok = new QPushButton( tr("Ok"), dialog );
    connect( ok, SIGNAL(clicked()), dialog, SLOT(accept()) );
    ok->setDefault(TRUE);
    cancel = new QPushButton( tr("Cancel"), dialog );
    connect( cancel, SIGNAL(clicked()), dialog, SLOT(reject()) );
    buttons->addWidget( ok );
    buttons->addWidget( cancel );
    buttons->addStretch();

    QVBoxLayout *rightLay = new QVBoxLayout;
    topLay->addLayout( rightLay );

    QHBoxLayout *pickLay = new QHBoxLayout;
    rightLay->addLayout( pickLay );

    cp = new QColorPicker( dialog );
    cp->setFrameStyle( QFrame::Panel + QFrame::Sunken );
    pickLay->addWidget( cp );

    lp = new QColorLuminancePicker( dialog );
    lp->setFixedWidth( 20 ); //###
    pickLay->addWidget( lp );

    connect( cp, SIGNAL(newCol(int,int)), lp, SLOT(setCol(int,int)) );
    connect( lp, SIGNAL(newHsv(int,int,int)), this, SLOT(newHsv(int,int,int)) );

    rightLay->addStretch();

    cs = new QColorShower( dialog );
    connect( cs, SIGNAL(newCol(QRgb)), this, SLOT(newColorTypedIn(QRgb)));
    rightLay->addWidget( cs );


    QPushButton *addCusBt = new QPushButton( tr("&Add To Custom Colors"),
					     dialog );
    rightLay->addWidget( addCusBt );
    connect( addCusBt, SIGNAL(clicked()), this, SLOT(addCustom()) );
}

void QColorDialogPrivate::addCustom()
{
    if ( nCust < 16 ) {
	cusrgb[nCust++] =  cs->currentColor();
	custom->repaint( FALSE ); //###
    }
}

QColorDialog::QColorDialog(QWidget* parent, const char* name, bool modal) :
    QDialog(parent, name, modal )
{
    d = new QColorDialogPrivate( this );
}



/*!
  Pops up a color dialog letting the user choose a color and returns
  that color. Returns an \link QColor::isValid() invalid\endlink color
  if the user cancels the dialog. All colors allocated by the dialog will be
  deallocated before this function returns.
*/

QColor QColorDialog::getColor( QWidget *parent, const char *name )
{
    int allocContext = QColor::enterAllocContext();
    QColorDialog *dlg = new QColorDialog( parent, name, TRUE );  //modal
    int resultCode = dlg->exec();
    QColor::leaveAllocContext();
    QColor result;
    if ( resultCode == QDialog::Accepted )
	result = dlg->selectedColor();
    QColor::destroyAllocContext(allocContext);

    return result;
}



/*!
  Returns the currently selected color of the dialog.
*/

QColor QColorDialog::selectedColor() const
{
    return QColor(d->currentColor());
}

#include "qcolordialog.moc"


