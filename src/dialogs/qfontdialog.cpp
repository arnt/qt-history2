/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qfontdialog.cpp#5 $
**
** C++ file skeleton
**
** Copyright (C) 1996 by Troll Tech AS.  All rights reserved.
**
*****************************************************************************/

#include "qfontdialog.h"

#include "qlined.h"
#include "qlistbox.h"
#include "qpushbt.h"
#include "qchkbox.h"
#include "qcombo.h"
#include "qlayout.h"
#include "qgrpbox.h"
#include "qlabel.h"
#include "qkeycode.h"
#include "qapp.h"

RCSTAG("$Id: //depot/qt/main/src/dialogs/qfontdialog.cpp#5 $");


struct QFontDialogPrivate
{
    QLabel * familyAccel;
    QLineEdit * familyEdit;
    QListBox * familyList;

    QLabel * styleAccel;
    QLineEdit * styleEdit;
    QListBox * styleList;

    QLabel * sizeAccel;
    QLineEdit * sizeEdit;
    QListBox * sizeList;

    QGroupBox * effects;
    QCheckBox * strikeout;
    QCheckBox * underline;
    QLabel * colorAccel;
    QComboBox * color;

    QGroupBox * sample;
    QLineEdit * sampleEdit;

    QLabel * scriptAccel;
    QComboBox * script;

    QPushButton * ok;
    QPushButton * cancel;

    QBoxLayout * buttonLayout;
    QBoxLayout * topLevelLayout;
    QBoxLayout * effectsLayout;
    QBoxLayout * sampleLayout;
    QBoxLayout * sampleEditLayout;
};


/*!  Creates a default font dialog.

*/

QFontDialog::QFontDialog( QWidget *parent, const char *name,
			  bool modal, WFlags f )
    : QDialog( parent, name, modal, f )
{
    // grid
    d->familyEdit = new QLineEdit( this, "font family I" );
    d->familyEdit->setFocusPolicy( StrongFocus );
    d->familyList = new QListBox( this, "font family II" );
    d->familyList->setFocusPolicy( NoFocus );
    d->familyList->setAutoScrollBar( FALSE );
    d->familyList->setScrollBar( TRUE );
    d->familyAccel
	= new QLabel( d->familyEdit, "&Font", this, "family accelerator" );
    d->familyAccel->setMargin( 2 );

    d->styleEdit = new QLineEdit( this, "font style I" );
    d->styleEdit->setFocusPolicy( StrongFocus );
    d->styleList = new QListBox( this, "font style II" );
    d->styleList->setFocusPolicy( NoFocus ); 
    d->styleList->setAutoScrollBar( FALSE );
    d->styleList->setScrollBar( TRUE );
    d->styleAccel
	= new QLabel( d->styleEdit, "Font st&yle", this, "style accelerator" );
    d->styleAccel->setMargin( 2 );

    d->sizeEdit = new QLineEdit( this, "font size I" );
    d->sizeEdit->setFocusPolicy( StrongFocus );
    d->sizeList = new QListBox( this, "font size II" );
    d->sizeList->setFocusPolicy( NoFocus );
    d->sizeList->setAutoScrollBar( FALSE );
    d->sizeList->setScrollBar( TRUE );
    d->sizeAccel
	= new QLabel ( d->sizeEdit, "&Size", this, "size accelerator" );
    d->sizeAccel->setMargin( 2 );

    // effects box
    d->effects = new QGroupBox( this, "font effects" );
    d->effects->setTitle( "Effects" );
    d->strikeout = new QCheckBox( d->effects, "strikeout on/off" );
    d->strikeout->setText( "Stri&keout" );
    d->underline = new QCheckBox( d->effects, "underline on/off" );
    d->underline->setText( "&Underline" );
    d->color = new QComboBox( TRUE, d->effects, "pen color" );
    d->colorAccel
	= new QLabel( d->color, "&Color", d->effects, "color label" );
    d->colorAccel->setMargin( 2 );

    d->effectsLayout = new QBoxLayout( d->effects, QBoxLayout::Down, 8 );
    d->effectsLayout->addSpacing( 12 );
    d->effectsLayout->addWidget( d->strikeout, 0, AlignLeft );
    d->effectsLayout->addWidget( d->underline, 0, AlignLeft );
    d->effectsLayout->addStretch( 2 );
    d->effectsLayout->addWidget( d->colorAccel, 0, AlignLeft );
    d->effectsLayout->addWidget( d->color );
    d->effectsLayout->addStretch( 2 );

    // sample and script box
    QWidget * sampleStuff = new QWidget( this, "sample and more, wrapped up" );

    d->sample = new QGroupBox( sampleStuff, "sample text" );
    d->sample->setTitle( "Sample" );
    d->sampleEdit = new QLineEdit( d->sample, "r/w sample text" );
    d->sampleEdit->setText( "AaBbYyZz" );
    d->script = new QComboBox( TRUE, sampleStuff, "font encoding" );
    d->script->setFocusPolicy( StrongFocus );
    d->scriptAccel
	= new QLabel( d->script, "Scr&ipt", sampleStuff, "encoding label" );
    d->scriptAccel->setMargin( 2 );

    d->sampleLayout = new QBoxLayout( sampleStuff, QBoxLayout::Down, 0 );
    d->sampleLayout->addWidget( d->sample, 2 );
    d->sampleLayout->addSpacing( 5 );
    d->sampleLayout->addStretch( 10 );
    d->sampleLayout->addWidget( d->scriptAccel, 0, AlignLeft );
    d->sampleLayout->addWidget( d->script, 0, AlignLeft );

    // layout for sampleEdit
    d->sampleEditLayout = new QBoxLayout( d->sample, QBoxLayout::Down, 12 );
    d->sampleEditLayout->addSpacing( 6 );
    d->sampleEditLayout->addWidget( d->sampleEdit );

    // top-level and grid layout
    d->topLevelLayout = new QBoxLayout( this, QBoxLayout::Down, 12 );
    QGridLayout * mainGrid = new QGridLayout( 5, 7, 0 );

    d->topLevelLayout->addLayout( mainGrid );

    mainGrid->addWidget( d->familyAccel, 0, 0 );
    mainGrid->addWidget( d->familyEdit, 1, 0 );
    mainGrid->addWidget( d->familyList, 2, 0 );

    mainGrid->addWidget( d->styleAccel, 0, 2 );
    mainGrid->addWidget( d->styleEdit, 1, 2 );
    mainGrid->addWidget( d->styleList, 2, 2 );

    mainGrid->addWidget( d->sizeAccel, 0, 4 );
    mainGrid->addWidget( d->sizeEdit, 1, 4 );
    mainGrid->addWidget( d->sizeList, 2, 4 );

    mainGrid->setRowStretch( 2, 4 );
    mainGrid->setRowStretch( 4, 2 );
    // next numbers must match with those in updateGeometry()
    mainGrid->setColStretch( 0, 94 );
    mainGrid->addColSpacing( 1, 14 );
    mainGrid->setColStretch( 2, 64 );
    mainGrid->addColSpacing( 3, 14 );
    mainGrid->setColStretch( 4, 32 );
    mainGrid->addColSpacing( 5, 14 );

    mainGrid->addRowSpacing( 3, 18 );

    mainGrid->addWidget( d->effects, 4, 0 );

    mainGrid->addMultiCellWidget( sampleStuff, 4, 4, 2, 4 );

    d->buttonLayout = new QBoxLayout( QBoxLayout::Down, 0 );
    mainGrid->addLayout( d->buttonLayout, 1, 6 );

    d->ok = new QPushButton( "OK", this, "accept font selection" );
    connect( d->ok, SIGNAL(clicked()), SLOT(accept()) );
    d->buttonLayout->addWidget( d->ok, 0, AlignLeft );

    d->buttonLayout->addSpacing( 6 );

    d->cancel = new QPushButton( "Cancel", this, "cancel" );
    connect( d->cancel, SIGNAL(clicked()), SLOT(reject()) );
    d->buttonLayout->addWidget( d->cancel, 0, AlignLeft );

    d->buttonLayout->addStretch( 1 );

    connect( d->familyList, SIGNAL(highlighted(const char *)),
	     SLOT(familyHighlighted(const char *)) );
    connect( d->styleList, SIGNAL(highlighted(const char *)),
	     SLOT(styleHighlighted(const char *)) );
    connect( d->sizeList, SIGNAL(highlighted(const char *)),
	     SLOT(sizeHighlighted(const char *)) );

    connect( d->familyEdit, SIGNAL(returnPressed()),
	     SLOT(familySelected()) );
    connect( d->styleEdit, SIGNAL(returnPressed()),
	     SLOT(styleSelected()) );
    connect( d->sizeEdit, SIGNAL(returnPressed()),
	     SLOT(sizeSelected()) );

    resize( 1, 1 );
    updateGeometry();

    d->familyEdit->installEventFilter( this );
    d->styleEdit->installEventFilter( this );
    d->sizeEdit->installEventFilter( this );
    d->familyList->installEventFilter( this );
    d->styleList->installEventFilter( this );
    d->sizeList->installEventFilter( this );
}


/*! Deletes the font dialog and frees up its storage. */

QFontDialog::~QFontDialog()
{
    delete d;
    d = 0;
}


/*!  Display the font dialog, making sure that the contents are up to
  date.
*/

void QFontDialog::show()
{
    updateFontFamilies();
    if ( d->familyList->currentItem() < 0 )
	d->familyList->setCurrentItem( 0 );
    updateFontStyles();
    if ( d->styleList->currentItem() < 0 )
	d->styleList->setCurrentItem( 0 );
    updateFontSizes();
    if ( d->sizeList->currentItem() < 0 )
	d->sizeList->setCurrentItem( 0 );
    updateGeometry();
    QDialog::show();
}


/*! Returns a pointer to the "font families" list box.  This is usable
  mainly if you reimplement updateFontFamilies();
*/

QListBox * QFontDialog::fontFamilyListBox() const
{
    return d->familyList;
}


/*!  Update the contents of the "font families" list box.  This
  function can be reimplemented if you have special requirements.
*/

void QFontDialog::updateFontFamilies()
{
    QListBox * l = fontFamilyListBox();
    if ( l->count() == 0 ) {
	l->insertItem( "Times" );
	l->insertItem( "Helvetica" );
	l->insertItem( "Courier" );
	l->insertItem( "Palatino" );
	l->insertItem( "Gill Sans" );
    }
}


/*! Returns a pointer to the "font style" list box.  This is usable
  mainly if you reimplement updateFontStyles();
*/

QListBox * QFontDialog::fontStyleListBox() const
{
    return d->styleList;
}


/*!  Update the contents of the "font style" list box.  This
  function can be reimplemented if you have special requirements.
*/

void QFontDialog::updateFontStyles()
{
    QListBox * l = fontStyleListBox();
    if ( l->count() == 0 ) {
	l->insertItem( "Roman" );
	l->insertItem( "Italic" );
	l->insertItem( "Oblique" );
    }
}


/*! Returns a pointer to the "font size" list box.  This is usable
  mainly if you reimplement updateFontSizes();
*/

QListBox * QFontDialog::fontSizeListBox() const
{
    return d->sizeList;
}


/*!  Update the contents of the "font size" list box.  This
  function can be reimplemented if you have special requirements.
*/

void QFontDialog::updateFontSizes()
{
    QListBox * l = fontSizeListBox();
    if ( l->count() == 0 ) {
	l->insertItem( "10" );
	l->insertItem( "12" );
	l->insertItem( "14" );
	l->insertItem( "16" );
	l->insertItem( "18" );
	l->insertItem( "20" );
	l->insertItem( "24" );
	l->insertItem( "30" );
	l->insertItem( "36" );
	l->insertItem( "48" );
	l->insertItem( "72" );
    }
}


/*!  Update the available font styles and sizes to fit the newly
  highlighted family.
*/

void QFontDialog::familySelected()
{
    updateFontStyles();
    updateFontSizes();
}


/*!  Update the available font families and sizes to fit the newly
  highlighted style.
*/

void QFontDialog::styleSelected()
{
    updateFontFamilies();
    updateFontSizes();
}


/*!  Update the available font families and styles to fit the newly
  highlighted size.
*/

void QFontDialog::sizeSelected()
{
    updateFontFamilies();
    updateFontStyles();
}


/*!

*/

void QFontDialog::updateGeometry()
{
    int w1 = 188;
    int w2 = 128;
    int w3 = 64;

    int h1 = d->familyAccel->sizeHint().height();
    int h2 = d->familyEdit->sizeHint().height();
    int h3 = 108 - h2;

    d->familyAccel->setMinimumSize( w1, h1 );
    d->familyEdit->setMinimumSize( w1, h2 );
    d->familyList->setMinimumSize( w1, h3 );

    d->styleEdit->setMinimumSize( w2, h2 );
    d->sizeEdit->setMinimumSize( w3, h2 );

    d->familyAccel->setMaximumSize( QCOORD_MAX, h1 );
    d->familyEdit->setMaximumSize( QCOORD_MAX, h2 );

    QSize br( d->cancel->sizeHint() );
    if ( br.width() < 80 )
	br.setWidth( 80 );
    if ( br.height() < 28 )
	br.setHeight( 28 );
    d->ok->setFixedSize( br );
    d->cancel->setFixedSize( br );

    d->strikeout->setFixedSize( d->strikeout->sizeHint() );
    d->underline->setFixedSize( d->underline->sizeHint() );
    d->colorAccel->setFixedSize( d->colorAccel->sizeHint() );

    br = d->color->sizeHint();
    d->color->setMinimumSize( br );
    br.setWidth( QCOORD_MAX );
    d->color->setMaximumSize( br );

    d->sampleEdit->setMinimumHeight( 30 ); // ### deviates from the book
    d->scriptAccel->setFixedSize( d->scriptAccel->sizeHint() );

    br = d->script->sizeHint();
    d->script->setMinimumSize( br );
    br.setWidth( QCOORD_MAX );
    d->script->setMaximumSize( br );

    d->effectsLayout->activate();
    d->sampleLayout->activate();
    d->sampleEditLayout->activate();
    d->topLevelLayout->activate();
}


/*!  Event filter to make up, down, pageup and pagedown work correctly
  in the line edits.
*/

bool QFontDialog::eventFilter( QObject * o , QEvent * e )
{
    if ( !o || !e )
	return FALSE;


    if ( e->type() == Event_KeyPress ) {
    QListBox * lb = 0;
    QLineEdit * le = 0;

	if ( o == d->familyEdit ) {
	    lb = d->familyList;
	    le = d->familyEdit;
	} else if ( o == d->styleEdit ) {
	    lb = d->styleList;
	    le = d->styleEdit;
	} else if ( o == d->sizeEdit ) {
	    lb = d->sizeList;
	    le = d->sizeEdit;
	} else {
	    return FALSE;
	}
    
	QKeyEvent * k = (QKeyEvent *)e;
	if ( k->key() == Key_Up ||
	     k->key() == Key_Down ||
	     k->key() == Key_Prior ||
	     k->key() == Key_Next ) {
	    int ci = lb->currentItem();
	    (void)QApplication::sendEvent( lb, k );
	    if ( ci != lb->currentItem() && style() == WindowsStyle )
		le->selectAll();
	    return TRUE;
	}
    } else if ( e->type() == Event_FocusIn ) {
	if ( o == d->familyEdit )
	    d->familyEdit->selectAll();
	else if ( o == d->styleEdit )
	    d->styleEdit->selectAll();
	else if ( o == d->sizeEdit )
	    d->sizeEdit->selectAll();
    } else if ( e->type() == Event_MouseButtonPress ) {
	if ( o == d->familyList )
	    d->familyEdit->setFocus();
	else if ( o == d->styleList )
	    d->styleEdit->setFocus();
	else if ( o == d->sizeList )
	    d->sizeEdit->setFocus();
    }
    return FALSE;
}


/*!

*/

void QFontDialog::familyHighlighted( const char * t )
{
    d->familyEdit->setText( t );	
    if ( style() == WindowsStyle && d->familyEdit->hasFocus() )
	d->familyEdit->selectAll();
}


/*!

*/

void QFontDialog::styleHighlighted( const char * t )
{
    d->styleEdit->setText( t );	
    if ( style() == WindowsStyle && d->styleEdit->hasFocus() )
	d->styleEdit->selectAll();
}


/*!

*/

void QFontDialog::sizeHighlighted( const char * t )
{
    d->sizeEdit->setText( t );
    if ( style() == WindowsStyle && d->sizeEdit->hasFocus() )
	d->sizeEdit->selectAll();
}
