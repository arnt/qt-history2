/****************************************************************************
** Form implementation generated from reading ui file '/home/mark/p4/qt/tools/designer/manual/sgml/eg/credit/creditformbase.ui'
**
** Created: Tue Feb 20 15:30:58 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#include "/home/mark/p4/qt/tools/designer/manual/sgml/eg/credit/creditformbase.h"

#include <qvariant.h>   // first for gcc 2.7.2
#include <qbuttongroup.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qspinbox.h>
#include <qmime.h>
#include <qdragobject.h>
#include <qlayout.h>
#include <qtooltip.h>
#include <qwhatsthis.h>
#include <qimage.h>
#include <qpixmap.h>

static QPixmap uic_load_pixmap_CreditFormBase( const QString &name )
{
    const QMimeSource *m = QMimeSourceFactory::defaultFactory()->data( name );
    if ( !m )
	return QPixmap();
    QPixmap pix;
    QImageDrag::decode( m, pix );
    return pix;
}
/* 
 *  Constructs a CreditFormBase which is a child of 'parent', with the 
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  TRUE to construct a modal dialog.
 */
CreditFormBase::CreditFormBase( QWidget* parent,  const char* name, bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )
{
    if ( !name )
	setName( "CreditFormBase" );
    resize( 284, 201 ); 
    setCaption( tr( "Credit Rating" ) );
    CreditFormBaseLayout = new QVBoxLayout( this ); 
    CreditFormBaseLayout->setSpacing( 6 );
    CreditFormBaseLayout->setMargin( 11 );

    creditButtonGroup = new QButtonGroup( this, "creditButtonGroup" );
    creditButtonGroup->setTitle( tr( "Credit Rating" ) );
    creditButtonGroup->setColumnLayout(0, Qt::Vertical );
    creditButtonGroup->layout()->setSpacing( 0 );
    creditButtonGroup->layout()->setMargin( 0 );
    creditButtonGroupLayout = new QHBoxLayout( creditButtonGroup->layout() );
    creditButtonGroupLayout->setAlignment( Qt::AlignTop );
    creditButtonGroupLayout->setSpacing( 6 );
    creditButtonGroupLayout->setMargin( 11 );

    Layout1 = new QVBoxLayout; 
    Layout1->setSpacing( 6 );
    Layout1->setMargin( 0 );

    stdRadioButton = new QRadioButton( creditButtonGroup, "stdRadioButton" );
    stdRadioButton->setText( tr( "&Standard" ) );
    stdRadioButton->setChecked( TRUE );
    Layout1->addWidget( stdRadioButton );

    noneRadioButton = new QRadioButton( creditButtonGroup, "noneRadioButton" );
    noneRadioButton->setText( tr( "&None" ) );
    Layout1->addWidget( noneRadioButton );

    specialRadioButton = new QRadioButton( creditButtonGroup, "specialRadioButton" );
    specialRadioButton->setText( tr( "Sp&ecial" ) );
    Layout1->addWidget( specialRadioButton );
    creditButtonGroupLayout->addLayout( Layout1 );
    CreditFormBaseLayout->addWidget( creditButtonGroup );

    amountSpinBox = new QSpinBox( this, "amountSpinBox" );
    amountSpinBox->setEnabled( FALSE );
    amountSpinBox->setPrefix( tr( "$ " ) );
    amountSpinBox->setButtonSymbols( QSpinBox::UpDownArrows );
    amountSpinBox->setMaxValue( 100000 );
    amountSpinBox->setLineStep( 10000 );
    CreditFormBaseLayout->addWidget( amountSpinBox );

    Layout4 = new QHBoxLayout; 
    Layout4->setSpacing( 6 );
    Layout4->setMargin( 0 );
    QSpacerItem* spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout4->addItem( spacer );

    okPushButton = new QPushButton( this, "okPushButton" );
    okPushButton->setText( tr( "OK" ) );
    okPushButton->setDefault( TRUE );
    Layout4->addWidget( okPushButton );
    QSpacerItem* spacer_2 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout4->addItem( spacer_2 );

    cancelPushButton = new QPushButton( this, "cancelPushButton" );
    cancelPushButton->setText( tr( "Cancel" ) );
    Layout4->addWidget( cancelPushButton );
    QSpacerItem* spacer_3 = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    Layout4->addItem( spacer_3 );
    CreditFormBaseLayout->addLayout( Layout4 );





    // signals and slots connections
    connect( okPushButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( cancelPushButton, SIGNAL( clicked() ), this, SLOT( reject() ) );
    connect( creditButtonGroup, SIGNAL( clicked(int) ), this, SLOT( setAmount() ) );
    connect( specialRadioButton, SIGNAL( toggled(bool) ), amountSpinBox, SLOT( setEnabled(bool) ) );
    init();
}

/*  
 *  Destroys the object and frees any allocated resources
 */
CreditFormBase::~CreditFormBase()
{
    destroy();
    // no need to delete child widgets, Qt does it all for us
}

void CreditFormBase::setAmount()
{
    qWarning( "CreditFormBase::setAmount(): Not implemented yet!" );
}

void CreditFormBase::destroy()
{
}

void CreditFormBase::init()
{
}

