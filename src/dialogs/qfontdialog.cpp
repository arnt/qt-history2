/****************************************************************************
** $Id: //depot/qt/main/src/dialogs/qfontdialog.cpp#25 $
**
** Implementation of QFontDialog
**
** Created : 970605
**
** Copyright (C) 1992-1999 Troll Tech AS.  All rights reserved.
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

#include "qfontdialog.h"

#include "qlineedit.h"
#include "qlistbox.h"
#include "qpushbutton.h"
#include "qcheckbox.h"
#include "qcombobox.h"
#include "qlayout.h"
#include "qgroupbox.h"
#include "qlabel.h"
#include "qapplication.h"
#include "qfontdatabase.h"

//
//  W A R N I N G
//  -------------
//
//  This class is under development and is currently unstable.
//
//  It is very unlikely that this code will be available in the final
//  Qt 2.0 release.  It will be available soon after then, but a number
//  of important API changes still need to be made.
//
//  Thus, it is important that you do NOT use this code in an application
//  unless you are willing for your application to be dependent on the
//  snapshot releases of Qt.
//

/*!
  \class QFontDialog qfontdialog.h
  \brief The QFontDialog provides a dialog widget for selecting a text font
  \ingroup dialogs

  Use for allowing the user to select a font among the available fonts
  on the underlying window system.
*/

struct QFontDialogPrivate
{
    QFontDialogPrivate(){};
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
    QComboBox * scriptCombo;

    QPushButton * ok;
    QPushButton * cancel;

    QBoxLayout * buttonLayout;
    QBoxLayout * effectsLayout;
    QBoxLayout * sampleLayout;
    QBoxLayout * sampleEditLayout;

    QFontDatabase fdb;
    QFontFamily   family;
    QFontCharSet  charSet;
    QFontStyle    style;

};

/*
  Work around bugs in layout.
 */

class QExpandingLineEdit : public QLineEdit
{
public:
    QExpandingLineEdit( QWidget *parent, const char *name, bool fixedHeight )
	: QLineEdit( parent, name ) { f = fixedHeight; }
    QSizePolicy sizePolicy() const
    {
	    return QSizePolicy( QSizePolicy::Expanding,
				f ? QSizePolicy::Fixed :
				    QSizePolicy::Expanding  );
    }
    QSize sizeHint() const { QSize sz = QLineEdit::sizeHint();
                             sz.setWidth( 20 );
			     return sz;
                           }
    bool f;
};

class QExpandingComboBox : public QComboBox
{
public:
    QExpandingComboBox( bool b, QWidget *parent, const char *name=0 )
	: QComboBox( b, parent, name ) {}
    QSizePolicy sizePolicy() const
	{
	    return QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed );
	}
};


/*!
  Creates a default font dialog.
*/

QFontDialog::QFontDialog( QWidget *parent, const char *name,
			  bool modal, WFlags f )
    : QDialog( parent, name, modal, f )
{
    d = new QFontDialogPrivate;
    // grid
    d->familyEdit = new QExpandingLineEdit( this, "font family I", TRUE );
    d->familyEdit->setFocusPolicy( StrongFocus );
    d->familyList = new QListBox( this, "font family II" );
    d->familyList->setFocusPolicy( NoFocus );

    d->familyAccel
	= new QLabel( d->familyEdit, "&Font", this, "family accelerator" );
    d->familyAccel->setMargin( 2 );

    d->styleEdit = new QExpandingLineEdit( this, "font style I", TRUE );
    d->styleEdit->setFocusPolicy( StrongFocus );
    d->styleList = new QListBox( this, "font style II" );
    d->styleList->setFocusPolicy( NoFocus );
    d->styleAccel
	= new QLabel( d->styleEdit, "Font st&yle", this, "style accelerator" );
    d->styleAccel->setMargin( 2 );

    d->sizeEdit = new QExpandingLineEdit( this, "font size I", TRUE );
    d->sizeEdit->setFocusPolicy( StrongFocus );
    d->sizeList = new QListBox( this, "font size II" );
    d->sizeList->setFocusPolicy( NoFocus );
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
    d->color->setEnabled( FALSE );
    d->colorAccel
	= new QLabel( d->color, "&Color", d->effects, "color label" );
    d->colorAccel->setMargin( 2 );

    d->effectsLayout = new QBoxLayout( d->effects, QBoxLayout::Down, 6, 0 );
    d->effectsLayout->addSpacing( 12 );
    d->effectsLayout->addWidget( d->strikeout, 0, AlignLeft );
    d->effectsLayout->addSpacing( 2 );
    d->effectsLayout->addWidget( d->underline, 0, AlignLeft );
    //d->effectsLayout->addSpacing( 6 );
    d->effectsLayout->addWidget( d->colorAccel, 0, AlignLeft );
    d->effectsLayout->addWidget( d->color );
    //d->effectsLayout->addSpacing( 6 );

    // sample and script box
    QWidget * sampleStuff = new QWidget( this, "sample and more, wrapped up" );

    d->sample = new QGroupBox( sampleStuff, "sample text" );
    d->sample->setTitle( "Sample" );
    d->sampleEdit = new QExpandingLineEdit( d->sample, "r/w sample text", FALSE );
    d->sampleEdit->setText( "AaBbYyZz" );
    d->scriptCombo = new QExpandingComboBox( TRUE, sampleStuff, "font encoding" );
    d->scriptCombo->setFocusPolicy( StrongFocus );
    d->scriptAccel
	= new QLabel( d->scriptCombo, "Scr&ipt", sampleStuff,"encoding label");
    d->scriptAccel->setMargin( 2 );

    d->sampleLayout = new QBoxLayout( sampleStuff, QBoxLayout::Down, 0 );
    d->sampleLayout->addWidget( d->sample, 2 );
    d->sampleLayout->addSpacing( 5 );
    //    d->sampleLayout->addStretch( 10 );
    d->sampleLayout->addWidget( d->scriptAccel, 0, AlignLeft );
    d->sampleLayout->addWidget( d->scriptCombo, 0, AlignLeft );

    // layout for sampleEdit
    d->sampleEditLayout = new QBoxLayout( d->sample, QBoxLayout::Down, 12, 0 );
    d->sampleEditLayout->addSpacing( 6 );
    d->sampleEditLayout->addWidget( d->sampleEdit, 42 );

    connect( d->familyList, SIGNAL(highlighted(const QString&)),
	     SLOT(familyHighlighted(const QString&)) );
    connect( d->scriptCombo, SIGNAL(activated(const QString&)),
	     SLOT(scriptHighlighted(const QString&)) );
    connect( d->styleList, SIGNAL(highlighted(const QString&)),
	     SLOT(styleHighlighted(const QString&)) );
    connect( d->sizeList, SIGNAL(highlighted(const QString&)),
	     SLOT(sizeHighlighted(const QString&)) );

    connect( d->familyEdit, SIGNAL(returnPressed()),
	     SLOT(familySelected()) );
    connect( d->styleEdit, SIGNAL(returnPressed()),
	     SLOT(styleSelected()) );
    connect( d->sizeEdit, SIGNAL(returnPressed()),
	     SLOT(sizeSelected()) );

    connect( d->strikeout, SIGNAL(clicked()),
	     SLOT(updateSample()) );
    connect( d->underline, SIGNAL(clicked()),
	     SLOT(updateSample()) );

    updateFamilies();
    QSize sz;
    sz = d->familyList->sizeHint();
    warning( "Xfamily(%i, %i)", sz.width(), sz.height() );
    sz = d->styleList->sizeHint();
    warning( "Xstyle(%i, %i)", sz.width(), sz.height() );
    sz = d->sizeList->sizeHint();
    warning( "Xsize(%i, %i)", sz.width(), sz.height() );

    // grid layout
    QGridLayout * mainGrid = new QGridLayout( this, 5, 7, 12, 0 );

    mainGrid->addWidget( d->familyAccel, 0, 0 );
    mainGrid->addWidget( d->familyEdit, 1, 0 );
    mainGrid->addWidget( d->familyList, 2, 0 );

    mainGrid->addWidget( d->styleAccel, 0, 2 );
    mainGrid->addWidget( d->styleEdit, 1, 2 );
    mainGrid->addWidget( d->styleList, 2, 2 );

    mainGrid->addWidget( d->sizeAccel, 0, 4 );
    mainGrid->addWidget( d->sizeEdit, 1, 4 );
    mainGrid->addWidget( d->sizeList, 2, 4 );

    mainGrid->setColStretch( 0, 38 );
    mainGrid->setColStretch( 2, 24 );
    mainGrid->setColStretch( 4, 10 );

    mainGrid->addColSpacing( 1, 6 );
    mainGrid->addColSpacing( 3, 6 );
    mainGrid->addColSpacing( 5, 6 );

    mainGrid->addRowSpacing( 3, 12 );

    mainGrid->addWidget( d->effects, 4, 0 );

    mainGrid->addMultiCellWidget( sampleStuff, 4, 4, 2, 4 );

    QWidget *buttonBox = new QWidget( this, "button box" );
    mainGrid->addMultiCellWidget( buttonBox, 1, 4, 6, 6 );

    d->buttonLayout = new QBoxLayout( buttonBox, QBoxLayout::Down, 0 );

    d->ok = new QPushButton( "OK", buttonBox, "accept font selection" );
    connect( d->ok, SIGNAL(clicked()), SLOT(accept()) );
    d->buttonLayout->addWidget( d->ok, 0, AlignLeft );

    d->buttonLayout->addSpacing( 6 );

    d->cancel = new QPushButton( "Cancel", buttonBox, "cancel" );
    connect( d->cancel, SIGNAL(clicked()), SLOT(reject()) );
    d->buttonLayout->addWidget( d->cancel, 0, AlignLeft );

    resize( 500, 360 );

    d->familyEdit->installEventFilter( this );
    d->styleEdit->installEventFilter( this );
    d->sizeEdit->installEventFilter( this );
    d->familyList->installEventFilter( this );
    d->styleList->installEventFilter( this );
    d->sizeList->installEventFilter( this );

    d->familyEdit->setFocus();
}

/*! Deletes the font dialog and frees up its storage. */

QFontDialog::~QFontDialog()
{
    delete d;
    d = 0;
}

/*!
  Opens a modal file dialog and returns the font selected by the user.

  \a def is the default selected font.

  The \a ok parameter is set to TRUE if the user clicked OK, and FALSE if
  the user clicked Cancel.

  If the user clicks Cancel the \a def font is returned.

  This static function is less capable than the full QFontDialog object,
  but is convenient and easy to use.

  Example:
  \code
    // start at the current working directory and with *.cpp as filter
    bool ok;
    QFont f = QFontDialog::getFont( &ok, QFont( "Times", 12 ), this );
    if ( ok ) {
        // the user selected a valid font
    } else {
        // the user cancelled the dialog
    }
  \endcode

*/
QFont QFontDialog::getFont( bool *ok, const QFont &def,
			    QWidget *parent, const char* name)
{
#if 1
    return getFont( ok, &def, parent, name );
#else
    return getFont( ok, 0, parent, name );
#endif
}

/*!
  Opens a modal file dialog and returns the font selected by the user.

  The \a ok parameter is set to TRUE if the user clicked OK, and FALSE if
  the user clicked Cancel.

  If the user clicks Cancel the Qt default font is returned.

  This static function is less capable than the full QFontDialog object,
  but is convenient and easy to use.

  Example:
  \code
    // start at the current working directory and with *.cpp as filter
    bool ok;
    QFont f = QFontDialog::getFont( &ok, this );
    if ( ok ) {
        // the user selected a valid font
    } else {
        // the user cancelled the dialog
    }
  \endcode

*/
QFont QFontDialog::getFont( bool *ok, QWidget *parent,const char* name)
{
    return getFont( ok, 0, parent, name );
}

QFont QFontDialog::getFont( bool *ok, const QFont *def,
			    QWidget *parent, const char* name)
{
    QFont result;
    if ( def )
	result = *def;

    QFontDialog *dlg = new QFontDialog( parent, name, TRUE );
    if ( def )
    dlg->setFont( *def );
    dlg->setCaption( "Font" );
    if ( dlg->exec() == QDialog::Accepted ) {
	result = dlg->font();
	if ( ok )
	    *ok = TRUE;
    } else {
	if ( ok )
	    *ok = FALSE;
    }
    delete dlg;
    return result;
}

/*! Returns a pointer to the "font families" list box.  This is usable
  mainly if you reimplement updateFontFamilies();
*/

QListBox * QFontDialog::familyListBox() const
{
    return d->familyList;
}

/*! Returns a pointer to the "font style" list box.  This is usable
  mainly if you reimplement updateFontStyles();
*/

QListBox * QFontDialog::styleListBox() const
{
    return d->styleList;
}

/*! Returns a pointer to the "font style" list box.  This is usable
  mainly if you reimplement updateFontStyles();
*/

QComboBox * QFontDialog::scriptCombo() const
{
    return d->scriptCombo;
}

/*! Returns a pointer to the "font size" list box.  This is usable
  mainly if you reimplement updateFontSizes();
*/

QListBox * QFontDialog::sizeListBox() const
{
    return d->sizeList;
}

void QFontDialog::scriptSelected()
{
}

/*!  Update the available font styles and sizes to fit the newly
  highlighted family.
*/

void QFontDialog::familySelected()
{
}

/*!  Update the available sizes to fit the newly
  highlighted style.
*/

void QFontDialog::styleSelected()
{
}


void QFontDialog::sizeSelected()
{
}


/*!  Event filter to make up, down, pageup and pagedown work correctly
  in the line edits.
*/

bool QFontDialog::eventFilter( QObject * o , QEvent * e )
{
    if ( !o || !e )
	return FALSE;


    if ( e->type() == QEvent::KeyPress ) {
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
    } else if ( e->type() == QEvent::FocusIn && style() == WindowsStyle ) {
	if ( o == d->familyEdit )
	    d->familyEdit->selectAll();
	else if ( o == d->styleEdit )
	    d->styleEdit->selectAll();
	else if ( o == d->sizeEdit )
	    d->sizeEdit->selectAll();
    } else if ( e->type() == QEvent::MouseButtonPress ) {
	if ( o == d->familyList )
	    d->familyEdit->setFocus();
	else if ( o == d->styleList )
	    d->styleEdit->setFocus();
	else if ( o == d->sizeList )
	    d->sizeEdit->setFocus();
    }
    return FALSE;
}



/*!  Update the contents of the "font script" combo box.  This
  function can be reimplemented if you have special requirements.
*/

void QFontDialog::updateFamilies()
{
    d->familyList->insertStrList( d->fdb.familyNames() );
    if ( d->familyList->count() != 0 )
	d->familyList->setCurrentItem( 0 );
}

/*!  Update the contents of the "font script" combo box.  This
  function can be reimplemented if you have special requirements.
*/

void QFontDialog::updateScripts()
{
    d->scriptCombo->clear();
    d->scriptCombo->insertStrList( d->family.charSetNames() );
    if ( d->scriptCombo->count() != 0 ) {
	// Avoid bug in QComboBox
	scriptHighlighted( d->scriptCombo->text( 0 ) );
    }
}

/*!  Update the contents of the "font style" list box.  This
  function can be reimplemented if you have special requirements.
*/
void QFontDialog::updateStyles()
{
    d->styleList->clear();
    d->styleList->insertStrList( d->charSet.styleNames() );
    if ( d->styleList->count() != 0 )
	d->styleList->setCurrentItem( 0 );  // Will call updateSizes
    // updateSizes();
}

void QFontDialog::updateSizes()
{
    QArray<int> sizes = d->style.pointSizes();
    d->sizeList->clear();
    int i;
    QString tmp;
    for( i = 0 ; i < d->style.nSizes() ; i++ ) {
	tmp.sprintf( "%i", sizes[i] );
	d->sizeList->insertItem( tmp );
    }
    if ( d->sizeList->count() != 0 )
	d->sizeList->setCurrentItem( 0 );
    d->sizeList->repaint();
}

/*! 

*/

void QFontDialog::familyHighlighted( const QString &s )
{
    d->familyEdit->setText( s );
    if ( style() == WindowsStyle && d->familyEdit->hasFocus() )
	d->familyEdit->selectAll();

    QFontFamily tmp = d->fdb.family( s );
    if ( tmp.isNull() ) {
	warning( "QFontDialog::updateFamilies: Internal error, cannot find family" );
	return;
    }
    d->family = tmp;
    updateScripts();
}

/*!

*/

void QFontDialog::scriptHighlighted( const QString &s )
{
    QFontCharSet tmp = d->family.charSet( s );
    if ( tmp.isNull() ) {
	warning( "QFontCharSet::updateScripts: Internal error, cannot find script." );
	return;
    }
    d->charSet = tmp;

    updateStyles();
}

/*!

*/

void QFontDialog::styleHighlighted( const QString &s )
{
    QFontStyle tmp = d->charSet.style( s );
    if ( tmp.isNull() ) {
	warning( "QFontCharSet::updateScripts: Internal error, cannot find script." );
	return;
    }
    d->style = tmp;

    d->styleEdit->setText( s );	
    if ( style() == WindowsStyle && d->styleEdit->hasFocus() )
	d->styleEdit->selectAll();
    updateSizes();
}


/*!

*/

void QFontDialog::sizeHighlighted( const QString &s )
{
    d->sizeEdit->setText( s );
    if ( style() == WindowsStyle && d->sizeEdit->hasFocus() )
	d->sizeEdit->selectAll();
    updateSample();
}

void QFontDialog::setFont( const QFont &f )
{
    // ### Quick hack
    familyHighlighted( f.family() );
    QFontCharSet charSet = d->family.charSet( f.charSet() );
    if ( charSet.isNull() )
	return;
    d->charSet = charSet;
    updateStyles();

#if 0
    QFontStyle style = QFontStyle( f );
    d->style = style;
#endif

    QString tmp;
    tmp.sprintf( "%i", f.pointSize() );
    d->sizeEdit->setText( tmp );
}

QFont QFontDialog::font() const
{
    QString tmp = d->sizeEdit->text();
    int pSize = tmp.toInt();
    if ( pSize == 0 )
	pSize = 12;
    QFont f = d->style.font( pSize );
    if ( d->strikeout->isChecked() )
	f.setStrikeOut( TRUE );
    if ( d->underline->isChecked() )
	f.setUnderline( TRUE );
    return f;
}

void QFontDialog::updateSample()
{
    d->sampleEdit->setFont( font() );
}
