/****************************************************************************
** Form implementation generated from reading ui file '/home/mark/p4/qt/tools/designer/manual/sgml/eg/dynamic/mainform.ui'
**
** Created: Mon Feb 19 17:50:12 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "/home/mark/p4/qt/tools/designer/manual/sgml/eg/dynamic/mainform.h"

#include <qvariant.h>   // first for gcc 2.7.2
#include <qlabel.h>
#include <qpushbutton.h>
#include <qwidgetfactory.h>
#include <qmime.h>
#include <qdragobject.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

static QPixmap uic_load_pixmap_MainForm( const QString &name )
{
    const QMimeSource *m = QMimeSourceFactory::defaultFactory()->data( name );
    if ( !m )
	return QPixmap();
    QPixmap pix;
    QImageDrag::decode( m, pix );
    return pix;
}
/* 
 *  Constructs a MainForm which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
MainForm::MainForm( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "MainForm" );
    resize( 401, 121 ); 
    setCaption( tr( "Main Form" ) );
    MainFormLayout = new QGridLayout( this ); 
    MainFormLayout->setSpacing( 6 );
    MainFormLayout->setMargin( 11 );

    quitPushButton = new QPushButton( this, "quitPushButton" );
    quitPushButton->setText( tr( "&Quit" ) );

    MainFormLayout->addWidget( quitPushButton, 0, 1 );

    creditPushButton = new QPushButton( this, "creditPushButton" );
    creditPushButton->setText( tr( "&Credit Dialog" ) );

    MainFormLayout->addWidget( creditPushButton, 0, 0 );

    TextLabel1 = new QLabel( this, "TextLabel1" );
    TextLabel1->setText( tr( "Credit Rating" ) );

    MainFormLayout->addWidget( TextLabel1, 1, 0 );

    ratingTextLabel = new QLabel( this, "ratingTextLabel" );
    ratingTextLabel->setText( tr( "Unrated" ) );

    MainFormLayout->addWidget( ratingTextLabel, 1, 1 );





    // signals and slots connections
    connect( creditPushButton, SIGNAL( clicked() ), this, SLOT( creditDialog() ) );
    connect( quitPushButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
    init();
}

/*  
 *  Destroys the object and frees any allocated resources
 */
MainForm::~MainForm()
{
    destroy();
    // no need to delete child widgets, Qt does it all for us
}

void MainForm::creditDialog()
{
    QDialog *creditForm = (QDialog *)
    	QWidgetFactory::create( "../credit/creditformbase.ui" );
    // Set up the dynamic dialog
    if ( creditForm->exec() ) {
	// The user accepted, act accordingly
    }
    delete creditForm;
}

void MainForm::destroy()
{
}

void MainForm::init()
{
}

