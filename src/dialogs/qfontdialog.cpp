/****************************************************************************
** $Id$
**
** Implementation of QFontDialog
**
** Created : 970605
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the dialogs module of the Qt GUI Toolkit.
**
** This file may be distributed under the terms of the Q Public License
** as defined by Trolltech AS of Norway and appearing in the file
** LICENSE.QPL included in the packaging of this file.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses may use this file in accordance with the Qt Commercial License
** Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/qpl/ for QPL licensing information.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qwindowdefs.h"

#ifndef QT_NO_FONTDIALOG

#include "qfontdialog.h"

#include "qlineedit.h"
#include "qlistbox.h"
#include "qpushbutton.h"
#include "qcheckbox.h"
#include "qcombobox.h"
#include "qlayout.h"
#include "qvgroupbox.h"
#include "qhgroupbox.h"
#include "qlabel.h"
#include "qapplication.h"
#include "qfontdatabase.h"
#include "qstyle.h"

#include <ctype.h>

/*!
  \class QFontDialog qfontdialog.h
  \ingroup dialogs
  \mainclass
  \brief The QFontDialog class provides a dialog widget for selecting a font.

  The usual way to use this class is to call one of the static convenience
  functions, getFont(), e.g.

  Examples:
  \code
    bool ok;
    QFont font = QFontDialog::getFont( &ok, QFont( "Helvetica [Cronyx]", 10 ), this );
    if ( ok ) {
	// font is set to the font the user selected
    } else {
	// the user cancelled the dialog; font is set to the initial
	// value, in this case Helvetica [Cronyx], 10
    }
  \endcode

    The dialog can also be used to set a widget's font directly:
  \code
    aWidget.setFont( QFontDialog::getFont( 0, aWidget.font() ) );
  \endcode
  If the user clicks OK the font they chose will be used for aWidget,
  and if they click cancel the original font is kept.

  \sa QFont QFontInfo QFontMetrics

  <img src=qfontdlg-m.png> <img src=qfontdlg-w.png>
*/

class QFontDialogPrivate
{
public:
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

    QVGroupBox * effects;
    QCheckBox * strikeout;
    QCheckBox * underline;
    QLabel * colorAccel;
    QComboBox * color;

    QHGroupBox * sample;
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
    QString       family;
    QString       script;
    QString       style;
    QString       size;

    QStringList familyNames;
    QStringList scriptNames;
    QValueList<int> scriptScripts;
    QStringList scriptSamples;
    bool usingStandardSizes;
};


/*!
  Constructs a common font dialog.

  Use setFont() for setting the initial font attributes.

    The \a parent, \a name, \a modal and \a f parameters are passed to
    the QDialog constructor.

  \sa getFont()
*/

QFontDialog::QFontDialog( QWidget *parent, const char *name,
			  bool modal, WFlags f )
    : QDialog( parent, name, modal, f )
{
    setSizeGripEnabled( TRUE );
    d = new QFontDialogPrivate;
    // grid
    d->familyEdit = new QLineEdit( this, "font family I" );
    d->familyEdit->setFocusPolicy( StrongFocus );
    d->familyList = new QListBox( this, "font family II" );
    d->familyList->viewport()->setFocusProxy( d->familyEdit );

    d->familyAccel
	= new QLabel( d->familyEdit, tr("&Font"), this, "family accelerator" );
    d->familyAccel->setIndent( 2 );

    d->styleEdit = new QLineEdit( this, "font style I" );
    d->styleEdit->setFocusPolicy( StrongFocus );
    d->styleList = new QListBox( this, "font style II" );
    d->styleList->viewport()->setFocusProxy( d->styleEdit );
    d->styleAccel
	= new QLabel( d->styleEdit, tr("Font st&yle"), this, "style accelerator" );
    d->styleAccel->setIndent( 2 );

    d->sizeEdit = new QLineEdit( this, "font size I" );
    d->sizeEdit->setFocusPolicy( StrongFocus );
    d->sizeList = new QListBox( this, "font size II" );
    d->sizeList->viewport()->setFocusProxy( d->sizeEdit );
    d->sizeAccel
	= new QLabel ( d->sizeEdit, tr("&Size"), this, "size accelerator" );
    d->sizeAccel->setIndent( 2 );

    // effects box
    d->effects = new QVGroupBox( tr("Effects"), this, "font effects" );
    d->strikeout = new QCheckBox( d->effects, "strikeout on/off" );
    d->strikeout->setText( tr("Stri&keout") );
    d->underline = new QCheckBox( d->effects, "underline on/off" );
    d->underline->setText( tr("&Underline") );

#if 0
    d->color = new QComboBox( TRUE, d->effects, "pen color" );
    d->color->setEnabled( FALSE );
    d->colorAccel
	= new QLabel( d->color, tr("&Color"), d->effects, "color label" );
    d->colorAccel->setIndent( 2 );
#endif

    d->sample = new QHGroupBox( tr("Sample"), this, "sample text" );
    d->sampleEdit = new QLineEdit( d->sample, "r/w sample text" );
    d->sampleEdit->setSizePolicy( QSizePolicy( QSizePolicy::Ignored, QSizePolicy::Ignored) );

    // Note that the sample text is *not* translated with tr(), as the
    // characters used depend on the charset encoding.
    d->sampleEdit->setText( "AaBbYyZz" );

    d->sampleEdit->setAlignment( AlignCenter );

    d->scriptCombo = new QComboBox( TRUE, this, "font encoding" );
    d->scriptCombo->setFocusPolicy( StrongFocus );

    d->scriptAccel
	= new QLabel( d->scriptCombo, tr("Scr&ipt"), this,"encoding label");
    d->scriptAccel->setIndent( 2 );

    d->usingStandardSizes = FALSE;

#if 0
    connect( d->familyList, SIGNAL(highlighted(const QString&)),
	     SLOT(familyHighlighted(const QString&)) );
#else
    connect( d->familyList, SIGNAL(highlighted(int)),
	     SLOT(familyHighlighted(int)) );
#endif
    connect( d->scriptCombo, SIGNAL(activated(int)),
	     SLOT(scriptHighlighted(int)) );
    connect( d->styleList, SIGNAL(highlighted(const QString&)),
	     SLOT(styleHighlighted(const QString&)) );
    connect( d->sizeList, SIGNAL(highlighted(const QString&)),
	     SLOT(sizeHighlighted(const QString&)) );

    connect( d->sizeEdit, SIGNAL(textChanged( const QString &)),
	     SLOT(sizeChanged(const QString&)) );

    connect( d->strikeout, SIGNAL(clicked()),
	     SLOT(updateSample()) );
    connect( d->underline, SIGNAL(clicked()),
	     SLOT(updateSample()) );

    updateFamilies();
    if ( d->familyList->count() != 0 )
	d->familyList->setCurrentItem( 0 );

    QSize sz;
    sz = d->familyList->sizeHint();
    sz = d->styleList->sizeHint();
    sz = d->sizeList->sizeHint();

    // grid layout
    QGridLayout * mainGrid = new QGridLayout( this, 9, 6, 12, 0 );

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

    mainGrid->addMultiCellWidget( d->sample, 4, 7, 2, 4 );

    mainGrid->addWidget( d->scriptAccel, 5, 0 );
    mainGrid->addRowSpacing( 6, 2 );
    mainGrid->addWidget( d->scriptCombo, 7, 0 );

    mainGrid->addRowSpacing( 8, 12 );

    QHBoxLayout *buttonBox = new QHBoxLayout;
    mainGrid->addMultiCell( buttonBox, 9, 9, 0, 4 );

    buttonBox->addStretch( 1 );
    QString okt = modal ? tr("OK") : tr("Apply");
    d->ok = new QPushButton( okt, this, "accept font selection" );
    buttonBox->addWidget( d->ok );
    if ( modal )
	connect( d->ok, SIGNAL(clicked()), SLOT(accept()) );
    connect( d->ok, SIGNAL(clicked()), SLOT(emitSelectedFont()) );
    d->ok->setDefault( TRUE );
    d->ok->setFixedWidth( 80 );

    buttonBox->addSpacing( 12 );

    QString cancelt = modal ? tr("Cancel") : tr("Close");
    d->cancel = new QPushButton( cancelt, this, "cancel/close" );
    buttonBox->addWidget( d->cancel );
    connect( d->cancel, SIGNAL(clicked()), SLOT(reject()) );
    d->cancel->setFixedWidth( 80 );

    updateScripts();

    resize( 500, 360 );

    d->familyEdit->installEventFilter( this );
    d->styleEdit->installEventFilter( this );
    d->sizeEdit->installEventFilter( this );
    d->familyList->installEventFilter( this );
    d->styleList->installEventFilter( this );
    d->sizeList->installEventFilter( this );

    d->familyEdit->setFocus();
}

/*! Destroys the font dialog and frees up its storage. */

QFontDialog::~QFontDialog()
{
    delete d;
    d = 0;
}

/*!
  Executes a modal font dialog and returns a font.

  If the user clicks OK, the selected font is returned. If the user
  clicks Cancel, the \a initial font is returned.

  The dialog has parent \a parent and is called \a name.
  \a initial is the initial selected font.
  If the \a ok parameter is not-null, \e *\a ok is set to TRUE if the
  user clicked OK, and set to FALSE if the user clicked Cancel.

  This static function is less functional than the full QFontDialog
  object, but is convenient and easy to use.

  Examples:
  \code
    bool ok;
    QFont font = QFontDialog::getFont( &ok, QFont( "Times", 12 ), this );
    if ( ok ) {
	// font is set to the font the user selected
    } else {
	// the user cancelled the dialog; font is set to the initial
	// value, in this case Times, 12.
    }
  \endcode

    The dialog can also be used to set a widget's font directly:
  \code
    myWidget.setFont( QFontDialog::getFont( 0, myWidget.font() ) );
  \endcode
  In this example, if the user clicks OK the font they chose will be
  used, and if they click cancel the original font is kept.
*/
QFont QFontDialog::getFont( bool *ok, const QFont &initial,
			    QWidget *parent, const char* name)
{
    return getFont( ok, &initial, parent, name );
}

/*!
    \overload

  Executes a modal font dialog and returns a font.

  If the user clicks OK, the selected font is returned. If the user
  clicks Cancel, the Qt default font is returned.

  The dialog has parent \a parent and is called \a name.
  If the \a ok parameter is not-null, \e * \a ok is set to TRUE if the user
  clicked OK, and FALSE if the user clicked Cancel.

  This static function is less functional than the full QFontDialog
  object, but is convenient and easy to use.

  Example:
  \code
    bool ok;
    QFont font = QFontDialog::getFont( &ok, this );
    if ( ok ) {
	// font is set to the font the user selected
    } else {
	// the user cancelled the dialog; font is set to the default
	// application font, QApplication::font()
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
#ifndef QT_NO_WIDGET_TOPEXTRA
    dlg->setCaption( tr("Select Font") );
#endif
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

/*!
    Returns a pointer to the "font family" list box.  This is mainly
    useful mainly if you reimplement updateFontFamilies();
*/

QListBox * QFontDialog::familyListBox() const
{
    return d->familyList;
}

/*!
    Returns a pointer to the "font style" list box.  This is mainly
    useful if you reimplement updateFontStyles();
*/

QListBox * QFontDialog::styleListBox() const
{
    return d->styleList;
}

/*!
    Returns a pointer to the "font style" list box.  This is mainly
    useful if you reimplement updateFontStyles();
*/

QComboBox * QFontDialog::scriptCombo() const
{
    return d->scriptCombo;
}

/*!
    Returns a pointer to the "font size" list box.  This is mainly
    useful if you reimplement updateFontSizes();
*/

QListBox * QFontDialog::sizeListBox() const
{
    return d->sizeList;
}

/*!
    This slot is called if the user changes the font size.
    The size is passed in the \a s argument as a \e string.
*/

void QFontDialog::sizeChanged( const QString &s )
{
    bool ok = FALSE;
    if ( d->size != s ) {
	(void) s.toInt( &ok );
	if ( ok ) {
	    d->size = s;
	    updateSample();
	}
    }
}


/*!
    An event filter to make the Up, Down, PageUp and PageDown keys work
    correctly in the line edits. The source of the event is the object
    \a o and the event is \a e.
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
	    if ( ci != lb->currentItem() &&
		 style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this))
		le->selectAll();
	    return TRUE;
	}
    } else if ( e->type() == QEvent::FocusIn &&
		style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this)) {
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
    return QDialog::eventFilter( o, e );
}


#ifdef Q_WS_MAC
// #define SHOW_FONTS_IN_FAMILIES
#endif

#ifdef SHOW_FONTS_IN_FAMILIES
#include "qpainter.h"

class QListBoxFontText : public QListBoxText
{
    QFont cfont;
public:
    QListBoxFontText( const QString & text );
    ~QListBoxFontText() { }

    int	 height( const QListBox * ) const;
    int	 width( const QListBox * )  const;

protected:
    void  paint( QPainter * );
};

QListBoxFontText::QListBoxFontText( const QString & text )
    : QListBoxText(text), cfont(text)
{
}

int QListBoxFontText::height( const QListBox * ) const
{
    QFontMetrics fm(cfont);
    return QMAX( fm.lineSpacing() + 2, QApplication::globalStrut().height() );
}

int QListBoxFontText::width( const QListBox * )  const
{
    QFontMetrics fm(cfont);
    return QMAX( fm.width( text() ) + 6, QApplication::globalStrut().width() );
}

void QListBoxFontText::paint( QPainter *painter )
{
    painter->save();
    painter->setFont(cfont);
    QListBoxText::paint(painter);
    painter->restore();
}

#endif

/*!  Updates the contents of the "font family" list box.  This
  function can be reimplemented if you have special requirements.
*/

void QFontDialog::updateFamilies()
{
    d->familyNames = d->fdb.families();
    QStringList newList;
    QString s;
    QStringList::Iterator it = d->familyNames.begin();
#ifdef SHOW_FONTS_IN_FAMILIES
    int idx = 0;
#endif
    for( ; it != d->familyNames.end() ; it++ ) {
	s = *it;

#if 0
	if ( d->fdb.isSmoothlyScalable( *it ) )
	    newList.append( s + "(TT)" );
	else if ( d->fdb.isBitmapScalable( *it ) )
	    newList.append( s + "(BT)" );
	else
#endif
#ifdef SHOW_FONTS_IN_FAMILIES
	    d->familyList->insertItem(new QListBoxFontText(s), idx++);
#else
	newList.append( s );
#endif
    }
#ifndef SHOW_FONTS_IN_FAMILIES
    d->familyList->insertStringList( newList );
#endif
}

/*!  Updates the contents of the "font script" combo box.  This
  function can be reimplemented if you have special requirements.
*/

void QFontDialog::updateScripts()
{
    d->scriptCombo->clear();
    d->scriptNames.clear();
    d->scriptScripts.clear();

    QString scriptname;
    for (int i = 0; i < QFont::NScripts; i++) {
	scriptname = QFontDatabase::scriptName((QFont::Script) i);
	if (! scriptname.isEmpty()) {
	    d->scriptNames += scriptname;
	    d->scriptScripts += i;
	}
    }

    if ( d->scriptNames.isEmpty() ) {
#ifndef QT_NO_DEBUG
	qWarning( "QFontDialog::updateFamilies: Internal error, "
		  "no scripts for family \"%s\"",
		  (const char *) d->family );
#endif
	return;
    }

    QStringList::Iterator it = d->scriptNames.begin();
    for ( ; it != d->scriptNames.end() ; ++it )
	d->scriptCombo->insertItem( *it );
}

/*!  Updates the contents of the "font style" list box.  This
  function can be reimplemented if you have special requirements.
*/

void QFontDialog::updateStyles()
{
    d->styleList->clear();

    QStringList styles = d->fdb.styles( d->family );

    if ( styles.isEmpty() ) {
#ifndef QT_NO_DEBUG
	qWarning( "QFontDialog::updateFamilies: Internal error, "
		  "no styles for family \"%s\" with script \"%s\"",
		  (const char *) d->family, (const char *) d->script );
#endif
	return;
    }
    d->styleList->insertStringList( styles );
}

/*!  Updates the contents of the "font size" list box.  This
  function can be reimplemented if you have special requirements.
*/

void QFontDialog::updateSizes()
{
    //    usingStandardSizes = d->fdb.isScalable( d->family );

    d->sizeList->clear();
    QValueList<int> sizes = d->fdb.pointSizes( d->family,d->style );

    if ( sizes.isEmpty() ) {
#ifndef QT_NO_DEBUG
	qWarning( "QFontDialog::updateFamilies: Internal error, "
		  "no pointsizes for family \"%s\" with script \"%s\"\n"
		  "and style \"%s\"",
		  (const char *) d->family, (const char *) d->script,
		  (const char *) d->style );
#endif
	return;
    }
    int i;
    QString tmp;
    for( i = 0 ; (uint)i < sizes.count() ; i++ ) {
	tmp.sprintf( "%i", sizes[i] );
	d->sizeList->insertItem( tmp );
    }
}

/*!

*/

void QFontDialog::familyHighlighted( const QString &s )
{
    d->familyEdit->setText(d->familyList->text(d->familyList->currentItem()));
    if ( style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this) &&
	 d->familyEdit->hasFocus() )
	d->familyEdit->selectAll();

    d->family = s;
    if ( d->scriptCombo->count() != 0 )
	scriptHighlighted( d->scriptCombo->currentItem() );
}

void QFontDialog::familyHighlighted( int i )
{
    QString s = d->familyNames[i];
    familyHighlighted( s );
}

/*!

*/

void QFontDialog::scriptHighlighted( int index )
{
    scriptHighlighted( d->scriptNames[index] );
    d->sampleEdit->setText( d->fdb.scriptSample( (QFont::Script) d->scriptScripts[index] ) );
}

void QFontDialog::scriptHighlighted( const QString &s )
{
    d->script = s;
    QString currentStyle = d->styleList->currentText();

    updateStyles();
    if ( d->styleList->count() != 0 ) {
	for ( int i = 0 ; i < (int)d->styleList->count() ; i++ ) {
	    if ( currentStyle == d->styleList->text(i) ) {
		d->styleList->setCurrentItem( i ); // Will call styleHighlighted
		break;
	    }
	}
        if ( d->styleList->currentItem() == -1 )
	    d->styleList->setCurrentItem( 0 ); // Will call styleHighlighted
    }
}

/*!

*/

void QFontDialog::styleHighlighted( const QString &s )
{
    d->styleEdit->setText( s );
    if ( style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this) &&
	 d->styleEdit->hasFocus() )
	d->styleEdit->selectAll();

    d->style = s;

    if ( d->usingStandardSizes && d->fdb.isScalable( d->family ) ) {
	updateSample();
	return;
    }
    int pSize = d->size.toInt();
    updateSizes();
    QString tmp;
    if ( d->sizeList->count() != 0 ) {
	int i;
	for ( i = 0 ; i < (int)d->sizeList->count() - 1 ; i++ ) {
	    tmp = d->sizeList->text(i);
	    if ( tmp.toInt() >= pSize )
		break;
	}
	d->sizeList->setCurrentItem( i );
    }
}


/*!

*/

void QFontDialog::sizeHighlighted( const QString &s )
{
    d->sizeEdit->setText( s );
    if ( style().styleHint(QStyle::SH_FontDialog_SelectAssociatedText, this) &&
	 d->sizeEdit->hasFocus() )
	d->sizeEdit->selectAll();

    d->size = s;
    updateSample();
}

enum match_t { MATCH_NONE=0, MATCH_LAST_RESORT=1, MATCH_DEFAULT=2, MATCH_FAMILY=3 };

/*!
  Sets the font the QFontDialog highlights to font \a f.

  \sa font()
*/

void QFontDialog::setFont( const QFont &f )
{
    QListBoxText *dummy = new QListBoxText( d->familyList );
    d->familyList->blockSignals( TRUE );
    d->familyList->setCurrentItem( dummy );
    d->familyList->blockSignals( FALSE );
    QString foundryName1, familyName1, foundryName2, familyName2;
    int bestFamilyMatch = -1;
    match_t bestFamilyType = MATCH_NONE;

    QFontDatabase::parseFontName(f.family(), foundryName1, familyName1);

    QStringList::Iterator it;
    it = d->familyNames.begin();
    int i = 0;
    for( ; it != d->familyNames.end() ; ++it, ++i ) {

	QFontDatabase::parseFontName(*it, foundryName2, familyName2);

	//try to match..
	if (foundryName1 == foundryName2 && familyName1 == familyName2) {
	    d->familyList->setCurrentItem(i);
	    delete dummy;
	    dummy = 0;
	    i = -1;
	    break;
	}

	//and try some fall backs
	match_t type = MATCH_NONE;
	switch(bestFamilyType) {
	case MATCH_NONE:
	    if(familyName2 == f.lastResortFamily()) {
		type = MATCH_LAST_RESORT;
		break;
	    }
	case MATCH_LAST_RESORT:
	    if(familyName2 == f.defaultFamily()) {
		type = MATCH_DEFAULT;
		break;
	    }
	case MATCH_DEFAULT:
	    if(familyName2 == familyName1) {
		type = MATCH_FAMILY;
		break;
	    }
	case MATCH_FAMILY: //already got the best
	    break;
	}
	if(type != MATCH_NONE)
	{
	    bestFamilyType = type;
	    bestFamilyMatch = i;
	}
    }

    if (i != -1 && bestFamilyType != MATCH_NONE) {
	d->familyList->setCurrentItem(bestFamilyMatch);
    } else if ( dummy ) {
	d->familyList->setCurrentItem( 0 );
    }
    delete dummy;
    dummy = 0;

    QString styleString = d->fdb.styleString( f );
    if ( !styleString.isEmpty() && d->styleList->count() != 0 ) {
	for ( i = 0 ; i < (int)d->styleList->count() ; i++ ) {
	    if ( styleString == d->styleList->text(i) ) {
		d->styleList->setCurrentItem( i );
		break;
	    }
	}
    }

    if ( d->sizeList->count() != 0 ) {
	int pSize = f.pointSize();
	for ( i = 0 ; i < (int)d->sizeList->count() - 1 ; i++ ) {
	    QString tmp = d->sizeList->text(i);
	    if ( tmp.toInt() >= pSize )
		break;
	}
	d->sizeList->setCurrentItem( i );
    }

    d->strikeout->setChecked( f.strikeOut() );
    d->underline->setChecked( f.underline() );

    updateSample();
}

/*!
  Returns the font which the user has chosen.

  \sa setFont()
*/

QFont QFontDialog::font() const
{
    int pSize = d->size.toInt();
    if ( pSize == 0 )
	pSize = 12;
    QFont f = d->fdb.font( d->family, d->style, pSize );

    f.setStrikeOut( d->strikeout->isChecked() );
    f.setUnderline( d->underline->isChecked() );
    return f;
}

void QFontDialog::updateSample()
{
    d->sampleEdit->setFont( font() );
    emit fontHighlighted(font());
}

void QFontDialog::emitSelectedFont()
{
    emit fontSelected(font());
}

/*!
  \fn void QFontDialog::fontSelected( const QFont & font )

  This signal is emitted when the user has chosen a font and clicked OK.
  The font that was selected is passed in \a font.
*/

/*!
  \fn void QFontDialog::fontHighlighted( const QFont & font )

  This signal is emitted when the user changed a setting in the dialog.
  The font that is highlighted is passed in \a font.
*/

#endif
