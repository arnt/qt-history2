/****************************************************************************
** Form implementation generated from reading ui file 'multiclip.ui'
**
** Created: Wed Feb 14 13:43:47 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "./multiclip.h"

#include <qvariant.h>   // first for gcc 2.7.2
#include <qapplication.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qlcdnumber.h>
#include <qlineedit.h>
#include <qlistbox.h>
#include <qpushbutton.h>
#include <qmime.h>
#include <qdragobject.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

static QPixmap uic_load_pixmap_MulticlipForm( const QString &name )
{
    const QMimeSource *m = QMimeSourceFactory::defaultFactory()->data( name );
    if ( !m )
	return QPixmap();
    QPixmap pix;
    QImageDrag::decode( m, pix );
    return pix;
}
/* 
 *  Constructs a MulticlipForm which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
MulticlipForm::MulticlipForm( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "MulticlipForm" );
    resize( 593, 476 ); 
    setCaption( tr( "Multiclip" ) );
    MulticlipFormLayout = new QVBoxLayout( this ); 
    MulticlipFormLayout->setSpacing( 6 );
    MulticlipFormLayout->setMargin( 11 );

    Layout1 = new QHBoxLayout; 
    Layout1->setSpacing( 6 );
    Layout1->setMargin( 0 );

    TextLabel1 = new QLabel( this, "TextLabel1" );
    TextLabel1->setText( tr( "Current Clipping" ) );
    Layout1->addWidget( TextLabel1 );

    currentLineEdit = new QLineEdit( this, "currentLineEdit" );
    Layout1->addWidget( currentLineEdit );
    MulticlipFormLayout->addLayout( Layout1 );

    Layout19 = new QGridLayout; 
    Layout19->setSpacing( 6 );
    Layout19->setMargin( 0 );

    addPushButton = new QPushButton( this, "addPushButton" );
    addPushButton->setText( tr( "&Add Clipping" ) );

    Layout19->addWidget( addPushButton, 4, 1 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Minimum, QSizePolicy::Expanding );
    Layout19->addItem( spacer, 2, 1 );

    quitPushButton = new QPushButton( this, "quitPushButton" );
    quitPushButton->setText( tr( "&Quit" ) );

    Layout19->addWidget( quitPushButton, 7, 1 );

    TextLabel3 = new QLabel( this, "TextLabel3" );
    TextLabel3->setText( tr( "Length" ) );

    Layout19->addWidget( TextLabel3, 0, 1 );

    TextLabel2 = new QLabel( this, "TextLabel2" );
    TextLabel2->setText( tr( "Previous Clippings" ) );

    Layout19->addWidget( TextLabel2, 0, 0 );

    autoCheckBox = new QCheckBox( this, "autoCheckBox" );
    autoCheckBox->setText( tr( "A&uto Add Clippings" ) );

    Layout19->addWidget( autoCheckBox, 3, 1 );

    lengthLCDNumber = new QLCDNumber( this, "lengthLCDNumber" );

    Layout19->addWidget( lengthLCDNumber, 1, 1 );

    deletePushButton = new QPushButton( this, "deletePushButton" );
    deletePushButton->setText( tr( "&Delete Clipping" ) );

    Layout19->addWidget( deletePushButton, 6, 1 );

    clippingsListBox = new QListBox( this, "clippingsListBox" );

    Layout19->addMultiCellWidget( clippingsListBox, 1, 7, 0, 0 );

    copyPushButton = new QPushButton( this, "copyPushButton" );
    copyPushButton->setText( tr( "&Copy Previous" ) );

    Layout19->addWidget( copyPushButton, 5, 1 );
    MulticlipFormLayout->addLayout( Layout19 );





    // signals and slots connections
    connect( quitPushButton, SIGNAL( clicked() ), this, SLOT( accept() ) );

    // tab order
    setTabOrder( currentLineEdit, clippingsListBox );
    setTabOrder( clippingsListBox, autoCheckBox );
    setTabOrder( autoCheckBox, addPushButton );
    setTabOrder( addPushButton, copyPushButton );
    setTabOrder( copyPushButton, deletePushButton );
    setTabOrder( deletePushButton, quitPushButton );
}

/*  
 *  Destroys the object and frees any allocated resources
 */
MulticlipForm::~MulticlipForm()
{
    // no need to delete child widgets, Qt does it all for us
}

void MulticlipForm::init()
{ 
    lengthLCDNumber->setBackgroundColor( darkBlue ); 
    currentLineEdit->setFocus(); 
     
    cb = qApp->clipboard(); 
    connect( cb, SIGNAL( dataChanged() ), SLOT( dataChanged() ) ); 
    if ( cb->supportsSelection() ) 
	connect( cb, SIGNAL( selectionChanged() ), SLOT( selectionChanged() ) ); 
     
    dataChanged(); 
}

void MulticlipForm::destroy()
{
    qWarning( "MulticlipForm::destroy(): Not implemented yet!" );
}

void MulticlipForm::addClipping()
{ 
    QString text = currentLineEdit->text(); 
    if ( ! text.isEmpty() ) { 
	lengthLCDNumber->display( (int)text.length() ); 	 
	int i = 0; 
	for ( ; i < (int)clippingsListBox->count(); i++ ) { 
	    if ( clippingsListBox->text( i ) == text ) { 
		i = -1; // Do not add duplicates 
		break; 
	    } 
	} 
	if ( i != -1 )  
	    clippingsListBox->insertItem( text, 0 );   	     
    } 
}

void MulticlipForm::copyPrevious()
{ 
    if ( clippingsListBox->currentItem() != -1 ) { 
	cb->setText( clippingsListBox->currentText() ); 
	if ( cb->supportsSelection() ) { 
	    cb->setSelectionMode( TRUE ); 
	    cb->setText( clippingsListBox->currentText() ); 
	    cb->setSelectionMode( FALSE ); 
	} 
    } 
}

void MulticlipForm::dataChanged()
{ 
    QString text;  
    text = cb->text();   	 
    clippingChanged( text ); 
    if ( autoCheckBox->isChecked() ) 
	addClipping(); 
}

void MulticlipForm::deleteClipping()
{  
    clippingChanged( "" );  
    clippingsListBox->removeItem( clippingsListBox->currentItem() ); 
}

void MulticlipForm::selectionChanged()
{ 
    cb->setSelectionMode( TRUE );  
    dataChanged(); 
    cb->setSelectionMode( FALSE );  
}

void MulticlipForm::clippingChanged( const QString & s )
{ 
    currentLineEdit->setText( s ); 
    lengthLCDNumber->display( (int)s.length() );  
}

