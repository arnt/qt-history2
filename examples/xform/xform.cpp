/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <qapplication.h>
#include <qevent.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qpushbutton.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <q3buttongroup.h>
#include <qlcdnumber.h>
#include <qslider.h>
#include <qmenubar.h>
#include <qfontdialog.h>
#include <qlayout.h>
#include <qvbox.h>
#include <qwidgetstack.h>

#include <qpainter.h>
#include <qpixmap.h>
#include <qpicture.h>

#include <stdlib.h>

using namespace Qt;

class ModeNames {
public:
    enum Mode { Text, Image, Picture };
};


class XFormControl : public QVBox, public ModeNames
{
    Q_OBJECT
public:
    XFormControl( const QFont &initialFont, QWidget *parent=0, const char *name=0 );
   ~XFormControl() {}

    QWMatrix matrix();

signals:
    void newMatrix( QWMatrix );
    void newText( const QString& );
    void newFont( const QFont & );
    void newMode( int );
private slots:
    void newMtx();
    void newTxt(const QString&);
    void selectFont();
    void fontSelected( const QFont & );
    void changeMode(int);
    void timerEvent(QTimerEvent*);
private:
    Mode mode;
    QSlider	 *rotS;		       // Rotation angle scroll bar
    QSlider	 *shearS;	       // Shear value scroll bar
    QSlider	 *magS;		       // Magnification value scroll bar
    QLCDNumber	 *rotLCD;	       // Rotation angle LCD display
    QLCDNumber	 *shearLCD;	       // Shear value LCD display
    QLCDNumber	 *magLCD;	       // Magnification value LCD display
    QCheckBox	 *mirror;	       // Checkbox for mirror image on/of
    QWidgetStack* optionals;
    QLineEdit	 *textEd;	       // Inp[ut field for xForm text
    QPushButton  *fpb;		       // Select font push button
    QRadioButton *rb_txt;	       // Radio button for text
    QRadioButton *rb_img;	       // Radio button for image
    QRadioButton *rb_pic;	       // Radio button for picture
    QFont currentFont;
};

/*
  ShowXForm displays a text or a pixmap (QPixmap) using a coordinate
  transformation matrix (QWMatrix)
*/

class ShowXForm : public QWidget, public ModeNames
{
    Q_OBJECT
public:
    ShowXForm( const QFont &f, QWidget *parent=0, const char *name=0 );
   ~ShowXForm() {}

    Mode mode() const { return m; }
public slots:
    void setText( const QString& );
    void setMatrix( QWMatrix );
    void setFont( const QFont &f );
    void setPixmap( QPixmap );
    void setPicture( const QPicture& );
    void setMode( int );
private:
    QSizePolicy sizePolicy() const;
    QSize sizeHint() const;
    void paintEvent( QPaintEvent * );
    QWMatrix  mtx;			// coordinate transform matrix
    QString   text;			// text to be displayed
    QPixmap   pix;			// pixmap to be displayed
    QPicture  picture;			// text to be displayed
    Mode      m;
};

XFormControl::XFormControl( const QFont &initialFont,
			    QWidget *parent, const char *name )
	: QVBox( parent, name )
{
    setSpacing(6);
    setMargin(6);
    currentFont = initialFont;
    mode = Image;

    rotLCD	= new QLCDNumber( 4, this, "rotateLCD" );
    rotS	= new QSlider( Qt::Horizontal, this,
				  "rotateSlider" );
    shearLCD	= new QLCDNumber( 5,this, "shearLCD" );
    shearS	= new QSlider( Qt::Horizontal, this,
				  "shearSlider" );
    mirror	= new QCheckBox( this, "mirrorCheckBox" );
    rb_txt = new QRadioButton( this, "text" );
    rb_img = new QRadioButton( this, "image" );
    rb_pic = new QRadioButton( this, "picture" );
    optionals = new QWidgetStack(this);
    QVBox* optionals_text = new QVBox(optionals);
    optionals_text->setSpacing(6);
    QVBox* optionals_other = new QVBox(optionals);
    optionals_other->setSpacing(6);
    optionals->addWidget(optionals_text,0);
    optionals->addWidget(optionals_other,1);
    fpb		= new QPushButton( optionals_text, "text" );
    textEd	= new QLineEdit( optionals_text, "text" );
    textEd->setFocus();

    rotLCD->display( "  0'" );

    rotS->setRange( -180, 180 );
    rotS->setValue( 0 );
    connect( rotS, SIGNAL(valueChanged(int)), SLOT(newMtx()) );

    shearLCD->display( "0.00" );

    shearS->setRange( -25, 25 );
    shearS->setValue( 0 );
    connect( shearS, SIGNAL(valueChanged(int)), SLOT(newMtx()) );

    mirror->setText( tr("Mirror") );
    connect( mirror, SIGNAL(clicked()), SLOT(newMtx()) );

    Q3ButtonGroup *bg = new Q3ButtonGroup(this);
    bg->hide();
    bg->insert(rb_txt,0);
    bg->insert(rb_img,1);
    bg->insert(rb_pic,2);
    rb_txt->setText( tr("Text") );
    rb_img->setText( tr("Image") );
    rb_img->setChecked(TRUE);
    rb_pic->setText( tr("Picture") );
    connect( bg, SIGNAL(clicked(int)), SLOT(changeMode(int)) );

    fpb->setText( tr("Select font...") );
    connect( fpb, SIGNAL(clicked()), SLOT(selectFont()) );

    textEd->setText( "Troll" );
    connect( textEd, SIGNAL(textChanged(const QString&)),
		     SLOT(newTxt(const QString&)) );

    magLCD = new QLCDNumber( 4,optionals_other, "magLCD" );
    magLCD->display( "100" );
    magS = new QSlider( Qt::Horizontal, optionals_other,
			   "magnifySlider" );
    magS->setRange( 0, 800 );
    connect( magS, SIGNAL(valueChanged(int)), SLOT(newMtx()) );
    magS->setValue( 0 );
    connect( magS, SIGNAL(valueChanged(int)), magLCD, SLOT(display(int)));

    optionals_text->adjustSize();
    optionals_other->adjustSize();
    changeMode(Image);

    startTimer(20); // start an initial animation
}

void XFormControl::timerEvent(QTimerEvent *e)
{
    int v = magS->value();
    v = (v+2)+v/10;
    if ( v >= 200 ) {
	v = 200;
	killTimer(e->timerId());
    }
    magS->setValue(v);
}



/*
    Called whenever the user has changed one of the matrix parameters
    (i.e. rotate, shear or magnification)
*/
void XFormControl::newMtx()
{
    emit newMatrix( matrix() );
}

void XFormControl::newTxt(const QString& s)
{
    emit newText(s);
    changeMode(Text);
}

/*
    Calculates the matrix appropriate for the current controls,
    and updates the displays.
*/
QWMatrix XFormControl::matrix()
{
    QWMatrix m;
    if (mode != Text) {
	double magVal = 1.0*magS->value()/100;
	m.scale( magVal, magVal );
    }
    double shearVal = 1.0*shearS->value()/25;
    m.shear( shearVal, shearVal );
    m.rotate( rotS->value() );
    if ( mirror->isChecked() ) {
	m.scale( 1, -1 );
	m.rotate( 180 );
    }

    QString tmp;
    tmp.sprintf( "%1.2f", shearVal  );
    if ( shearVal >= 0 )
	tmp.insert( 0, " " );
    shearLCD->display( tmp );

    int rot = rotS->value();
    if ( rot < 0 )
	rot = rot + 360;
    tmp.sprintf( "%3i'", rot );
    rotLCD->display( tmp );
    return m;
}


void XFormControl::selectFont()
{
    bool ok;
    QFont f = QFontDialog::getFont( &ok, currentFont );
    if ( ok ) {
	currentFont = f;
	fontSelected( f );
    }
}

void XFormControl::fontSelected( const QFont &font )
{
    emit newFont( font );
    changeMode(Text);
}

/*
    Sets the mode - Text, Image, or Picture.
*/

void XFormControl::changeMode(int m)
{
    mode = (Mode)m;

    emit newMode( m );
    newMtx();
    if ( mode == Text ) {
	optionals->raiseWidget(0);
	rb_txt->setChecked(TRUE);
    } else {
	optionals->raiseWidget(1);
	if ( mode == Image )
	    rb_img->setChecked(TRUE);
	else
	    rb_pic->setChecked(TRUE);
    }
    qApp->flushX();
}

ShowXForm::ShowXForm( const QFont &initialFont,
		      QWidget *parent, const char *name )
	: QWidget( parent, name )
{
    setFont( initialFont );
    setBackgroundColor( white );
    m = Text;
}

QSizePolicy ShowXForm::sizePolicy() const
{
    return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
}

QSize ShowXForm::sizeHint() const
{
    return QSize(400,400);
}



void ShowXForm::setText( const QString& s )
{
    text = s;
    update();
}

void ShowXForm::setMatrix( QWMatrix w )
{
    mtx = w;
    update();
}

void ShowXForm::setFont( const QFont &f )
{
    m = Text;
    QWidget::setFont( f );
}

void ShowXForm::setPixmap( QPixmap pm )
{
    pix	 = pm;
    m    = Image;
    update();
}

void ShowXForm::setPicture( const QPicture& p )
{
    picture = p;
    m = Picture;
    update();
}

void ShowXForm::setMode( int mode )
{
    m = (Mode)mode;
}
void ShowXForm::paintEvent( QPaintEvent * )
{
    QWMatrix um;  // copy user specified transform
    um.translate( width()/2, height()/2 );	// 0,0 is center
    um = mtx * um;
    QPainter p(this);
    p.setWorldMatrix( um );
    switch ( mode() ) {
    case Text: {
	QRect br = p.fontMetrics().boundingRect( 0,0,0,0, Qt::AlignCenter, text );
	p.drawText( br, Qt::AlignCenter, text );
	break;
    }
    case Image:
	p.drawPixmap( -pix.width()/2, -pix.height()/2, pix );
	break;
    case Picture:
	// ### need QPicture::boundingRect()
	p.scale(0.25,0.25);
	p.translate(-230,-180);
	p.drawPicture( 0, 0, picture );
    }
}


/*
    Grand unifying widget, putting ShowXForm and XFormControl
    together.
*/

class XFormCenter : public QHBox, public ModeNames
{
    Q_OBJECT
public:
    XFormCenter( QWidget *parent=0, const char *name=0 );
public slots:
    void setFont( const QFont &f ) { sx->setFont( f ); }
    void newMode( int );
private:
    ShowXForm	*sx;
    XFormControl *xc;
};

void XFormCenter::newMode( int m )
{
    static bool first_i = TRUE;
    static bool first_p = TRUE;

    if ( sx->mode() == m )
	return;
    if ( m == Image && first_i ) {
	first_i = FALSE;
	QPixmap pm;
	if ( pm.load( "image.any" ) )
	    sx->setPixmap( pm );
	return;
    }
    if ( m == Picture && first_p ) {
	first_p = FALSE;
	QPicture p;
	if (p.load( "picture.any" ))
	    sx->setPicture( p );
	return;
    }
    sx->setMode(m);
}

XFormCenter::XFormCenter( QWidget *parent, const char *name )
    : QHBox( parent, name )
{
    QFont f( "Charter", 36, QFont::Bold );

    xc = new XFormControl( f, this );
    sx = new ShowXForm( f, this );
    setStretchFactor(sx,1);
    xc->setFrameStyle( QFrame::Panel | QFrame::Raised );
    xc->setLineWidth( 2 );
    connect( xc, SIGNAL(newText(const QString&)), sx,
		 SLOT(setText(const QString&)) );
    connect( xc, SIGNAL(newMatrix(QWMatrix)),
	     sx, SLOT(setMatrix(QWMatrix)) );
    connect( xc, SIGNAL(newFont(const QFont&)), sx,
		 SLOT(setFont(const QFont&)) );
    connect( xc, SIGNAL(newMode(int)), SLOT(newMode(int)) );
    sx->setText( "Troll" );
    newMode( Image );
    sx->setMatrix(xc->matrix());
}


int main( int argc, char **argv )
{
    QApplication a( argc, argv );

    XFormCenter *xfc = new XFormCenter;

    a.setMainWidget( xfc );
    xfc->setWindowTitle("Qt Example - XForm");
    xfc->show();
    return a.exec();
}

#include "xform.moc"		      // include metadata generated by the moc
