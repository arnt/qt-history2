#include "mdlg.h"

#include <qpushbt.h>
#include <qapp.h>
#include <qlayout.h>
#include <qfont.h>
#include <qlcdnum.h>

MDialog::MDialog()
{
    fs = 0;

    QMenuBar* mb = new QMenuBar( this );

    QBoxLayout *gm = new QBoxLayout( this, QBoxLayout::Down, 5 );





    QBoxLayout *buttons = new QBoxLayout( QBoxLayout::LeftToRight );
    gm->addLayout( buttons );

    QPushButton *pb = new QPushButton( "Set RO", this );
    pb->setFixedSize( pb->sizeHint() );
    buttons->addWidget( pb );

    QPushButton *fb = new QPushButton( "Set Font", this );
    fb->setFixedSize( fb->sizeHint() );
    buttons->addWidget( fb );

    QPushButton *xb = new QPushButton( "Urk", this );
    xb->setFixedSize( xb->sizeHint() );
    buttons->addWidget( xb );

    lcd = new QLCDNumber( this );
    lcd->setFixedSize( fb->size() );
    buttons->addWidget( lcd );
    
    buttons->addStretch();

    QPushButton *qb = new QPushButton( "Quit", this );
    qb->setFixedSize( qb->sizeHint() );
    buttons->addWidget( qb );


    pb->setToggleButton(TRUE);
    pb->setOn( FALSE );

    m = new QMultiLineEdit( this );
    m->setMinimumSize( 80, 100 );
    ((QWidget*)m)->setFont( QFont("times") );
    m->setFocus();
    gm->addWidget( m, 10 );


    QPopupMenu* p1 = new QPopupMenu;

    int id = p1->insertItem("New" );
    p1->setItemEnabled( id, FALSE );
    id = p1->insertItem("Open..." );
    p1->setItemEnabled( id, FALSE );
    id = p1->insertItem("Save" );
    p1->setItemEnabled( id, FALSE );
    id = p1->insertItem("Save As..." );
    p1->setItemEnabled( id, FALSE );
    p1->insertItem("Quit", qApp , SLOT(quit()) );
    mb->insertItem("&File", p1);

    QPopupMenu* p2 = new QPopupMenu;
    mb->insertItem("&Edit", p2 );
    id = p2->insertItem("Cut", m, SLOT(cut()) );
    id = p2->insertItem("Copy", m, SLOT(copyText()) );
    id = p2->insertItem("Paste", m, SLOT(paste()) );
    id = p2->insertItem("Clear", m, SLOT(clear()) );

#if 1
    id =  mb->insertItem("Three");
    mb->setItemEnabled( id, FALSE );
    id =  mb->insertItem("Four");
    mb->setItemEnabled( id, FALSE );
#endif
#if 1
    QPopupMenu* p3 = new QPopupMenu;
    id =  mb->insertItem("&Item", p3);
    id = p3->insertItem("Testing" );
    p3->setItemEnabled( id, FALSE );
    id = p3->insertItem("One" );
    p3->setItemEnabled( id, FALSE );
    id = p3->insertItem("Two" );
    p3->setItemEnabled( id, FALSE );
    id = p3->insertItem("Three" );
    p3->setItemEnabled( id, FALSE );
    //mb->setItemEnabled( id, FALSE );
#endif


#if 1
    id =  mb->insertItem("Gulp");
    id =  mb->insertItem("Resp");
    id =  mb->insertItem("Hets");
    id =  mb->insertItem("Tens");
    id =  mb->insertItem("Tijs");
    id =  mb->insertItem("Tufs");
    mb->insertSeparator();
    id =  mb->insertItem("Help");
#endif

#if 0
    QFont f("Helvetica", 24, QFont::Bold);
    mb->setFont( f );
#endif

    gm->setMenuBar( mb );

    resize( 200, 200 );

    gm->activate();

    connect( m, SIGNAL(textChanged()),  SLOT(countUp()));
    connect( qb, SIGNAL(clicked()), qApp, SLOT(quit()));
    connect( fb, SIGNAL(clicked()),  SLOT(newFont()));
    connect( xb, SIGNAL(clicked()),  SLOT(urk()));
    connect( pb, SIGNAL(toggled(bool)), m, SLOT(setReadOnly(bool)));
    //connect( pb, SIGNAL(toggled(bool)), m, SLOT(setOverwriteMode(bool)));
#if 1
    m->setText( "To be, or not to be: that is the question:\n"
	       "Whether 'tis nobler in the mind to suffer\n"
	       "The slings and arrows of outrageous fortune,\n"
	       "Or to take arms against a sea of troubles,\n"
	       "And by opposing end them?  To die: to sleep;\n"
	       "No more; and by a sleep to say we end\n"
	       "The heart-ache and the thousand natural shocks\n"
	       "That flesh is heir to, 'tis a consummation\n"
	       "Devoutly to be wish'd.  To die, to sleep;\n"
	       "To sleep: perchance to dream: ay, there's the rub;\n"
	       "For in that sleep of death what dreams may come\n"
	       "When we have shuffled off this mortal coil,\n" 
	       "Must give us pause. . ." );
#endif    
}

void MDialog::setFont( const QFont & f )
{
    ((QWidget*)m)->setFont( f );
}

void MDialog::newFont()
{
    if (!fs) {
	fs   = new FontSelect;
	//connect( fs, SIGNAL(destroyed()),    SLOT(selectFontDestroyed()) );
	connect( fs, SIGNAL(newFont(const QFont&)),
		     SLOT(setFont(const QFont&)) );
	fs->setGeometry( QRect( 100, 200, 380, 260 ) );
	fsUp = FALSE;
    }
    fsUp = !fsUp;
    if ( fsUp )
	fs->show();
    else
	fs->hide();
}

void MDialog::urk()
{
    m->setCursorPosition( 0, 0 );
}


const int yOff = 35;

FontSelect::FontSelect( QWidget *parent, const char *name)
    : QDialog( parent, name, 0 ), f( "Charter", 48, QFont::Bold )
{
    static const char *radios[] = {
    	"Light (25)", "Normal (50)", "DemiBold (63)",
	"Bold (75)", "Black (87)"
    };
    int i;

    fontInternal = new QWidget( this );
    fontInternal->setFont( f );
    fontInternal->hide();

    familyL    = new QLabel(	 this, "familyLabel" );
    sizeL      = new QLabel(	 this, "sizeLabel" );
    family     = new QLineEdit(	 this, "family" );
    sizeE      = new QLineEdit(	 this, "pointSize" );
    italic     = new QCheckBox(	 this, "italic" );
    underline  = new QCheckBox(	 this, "underline" );
    strikeOut  = new QCheckBox(	 this, "strikeOut" );
    apply      = new QPushButton( this, "apply" );
    sizeS      = new QScrollBar( QScrollBar::Horizontal, this,
			      "pointSizeScrollBar" );

    familyL->setGeometry( 10, yOff + 10, 100,20 );
    familyL->setText( "Family :" );

    sizeL->setGeometry( 10, yOff + 40, 100, 20 );
    sizeL->setText( "Point size :" );

    family->setGeometry( 110, yOff + 10, 100, 20 );
    family->setText( "Charter" );

    sizeE->setGeometry( 110, yOff + 40, 100, 20 );
    sizeE->setText( "48" );

    sizeS->setGeometry( 220, yOff + 40, 100, 20 );
    sizeS->setRange( 1, 100 );
    sizeS->setValue( 48 );
    sizeS->setTracking( FALSE );
    connect( sizeS, SIGNAL(valueChanged(int)), SLOT(newSize(int)) );
    connect( sizeS, SIGNAL(sliderMoved(int)),  SLOT(slidingSize(int)) );

    italic->setGeometry( 10, yOff + 70, 80, 20 );
    italic->setText( "Italic" );
    connect( italic, SIGNAL(clicked()), SLOT(newItalic()) );

    underline->setGeometry( 110, yOff + 70, 80, 20 );
    underline->setText( "Underline" );
    connect( underline, SIGNAL(clicked()), SLOT(newUnderline()) );

    strikeOut->setGeometry( 210, yOff + 70, 80, 20 );
    strikeOut->setText( "StrikeOut" );
    connect( strikeOut, SIGNAL(clicked()), SLOT(newStrikeOut()) );

    apply->setGeometry( 235, yOff + 10, 70, 20);
    apply->setText( "APPLY" );
    apply->setDefault( TRUE );
    connect( apply, SIGNAL(clicked()), SLOT(doApply()) );

    weight = new QButtonGroup( "Weight", this, "weightGroupBox" );
    weight->setGeometry( 10, yOff + 100, 120, 120 );
    connect( weight, SIGNAL(clicked(int)), SLOT(newWeight(int)) );
    QString wname;
    for( i = 0 ; i < 5 ; i++ ) {
	wname.sprintf("radioButton %i",i);
	rb[i] = new QRadioButton( weight, wname );
	rb[i]->setGeometry( 10, 15+i*20 , 95, 20 );
	rb[i]->setText( radios[i] );
    }
    rb[3]->setChecked( TRUE );

    static const char *familys[] = {
	"Charter", "Clean", "Courier", "Fixed",
	"Gothic", "Helvetica", "Lucida", "Lucidabright",
	"Lucidatypewriter", "Mincho",
	"New century Schoolbook",
	"Symbol", "Terminal", "Times", "Utopia", "Arial", 0
    };

    static const char *pSizes[]  = {
	"8", "10", "12", "14", "18", "24", "36", "48", "72", "96", 0
    };

    QPopupMenu *file = new QPopupMenu;
    file->insertItem( "Quit\tC-x" );

    QPopupMenu *family = new QPopupMenu;
    const char **tmp;
    tmp = familys;
    while( *tmp )
	family->insertItem( *tmp++ );

    QPopupMenu *pSize = new QPopupMenu;
    tmp = pSizes;
    while( *tmp )
	pSize->insertItem( *tmp++ );

    menu = new QMenuBar( this );
    menu->move( 0, 0 );
    menu->resize( 350, 30 );
    menu->insertItem( "File", file );
    menu->insertItem( "Family", family );
    menu->insertItem( "Point size", pSize );

    connect( file, SIGNAL(activated(int)), SLOT(fileActivated(int)) );
    connect( family, SIGNAL(activated(int)), SLOT(familyActivated(int)) );
    connect( pSize, SIGNAL(activated(int)), SLOT(pSizeActivated(int)) );

    static const char *mLabelStr[] = {
	"Family:", "Point size:", "Weight:", "Italic:"
    };

    mGroup = new QButtonGroup( this, "metricsGroupBox" );
    mGroup->setTitle( "Actual font" );
    mGroup->setGeometry(140, yOff + 100, 230, 100);
    for( i = 0 ; i < 4 ; i++ ) {
	wname.sprintf("MetricsLabel[%i][%i]",i,0);
	metrics[i][0] = new QLabel( mGroup, wname);
	metrics[i][0]->setGeometry(10, 15 + 20*i, 70, 20);
	metrics[i][0]->setText( mLabelStr[i] );

	wname.sprintf("MetricsLabel[%i][%i]",i,1);
	metrics[i][1] = new QLabel( mGroup, wname);
	metrics[i][1]->setGeometry(90, 15 + 20*i, 135, 20);
    }
    updateMetrics();
}

void FontSelect::newStrikeOut()
{
    f.setStrikeOut( strikeOut->isChecked() );
    doFont();
}

void FontSelect::doFont()
{
    QFont xyz = f;
    xyz.setPointSize( f.pointSize()+1 );
    xyz.setPointSize( f.pointSize() );
    fontInternal->setFont( xyz );
    updateMetrics();
    emit newFont( xyz );
}

void FontSelect::newUnderline()
{
    f.setUnderline( underline->isChecked() );
    doFont();
}

void FontSelect::newItalic()
{
    f.setItalic( italic->isChecked() );
    doFont();
}

void FontSelect::newFamily()
{
    f.setFamily( family->text() );
    doFont();
}

void FontSelect::newWeight( int id )
{
    switch( id ) {
	case 0 :
	    f.setWeight( QFont::Light );
	    break;
	case 1 :
	    f.setWeight( QFont::Normal );
	    break;
	case 2 :
	    f.setWeight( QFont::DemiBold );
	    break;
	case 3 :
	    f.setWeight( QFont::Bold );
	    break;
	case 4 :
	    f.setWeight( QFont::Black );
	    break;
	default:
	    return;
    }
    doFont();
}

void FontSelect::newSize( int value )
{
    QString tmp;
    tmp.sprintf("%i", value);
    sizeE->setText( tmp );
    f.setPointSize( value );
    doFont();
}

void FontSelect::slidingSize( int value )
{
    QString tmp;

    tmp.sprintf("%i", value);
    sizeE->setText( tmp );
}

void FontSelect::doApply()
{
    int sz = atoi( sizeE->text() );
    if ( sz > 100) {
	sizeS->blockSignals( TRUE );
	sizeS->setValue( 100 );
	sizeS->blockSignals( FALSE );
	f.setPointSize( sz );
    } else {
	sizeS->setValue( atoi( sizeE->text() ) );
    }
    f.setFamily( family->text() );
    doFont();
}

void FontSelect::fileActivated( int )
{
    qApp->quit();
}

void FontSelect::familyActivated( int id )
{
    family->setText( ((QPopupMenu*)sender())->text(id) );
    newFamily();
}

void FontSelect::pSizeActivated( int id )
{
    int value = atoi( ( (QPopupMenu*)sender())->text( id ) );
    sizeS->blockSignals( TRUE );
    sizeS->setValue( value );
    sizeS->blockSignals( FALSE );
    newSize( value );
}

void FontSelect::updateMetrics()
{
    QFontInfo fi = fontInternal->fontInfo();
    metrics[0][1]->setText( fi.family() );
    metrics[1][1]->setNum( fi.pointSize() );
    metrics[2][1]->setNum( fi.weight() );
    metrics[3][1]->setNum( fi.italic() );
}


void MDialog::countUp()
{
    lcd->display( lcd->value() + 1 );
}

