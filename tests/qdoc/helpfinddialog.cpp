#include "helpfinddialog.h"
#include <qlayout.h>
#include <qlabel.h>
#include <qcombobox.h>
#include <qcheckbox.h>
#include <qpushbutton.h>
#include <qevent.h>

HelpFindDialog::HelpFindDialog( QWidget *parent )
    : QDialog( parent, "", FALSE )
{
    QGridLayout *layout = new QGridLayout( this, 3, 2 );
    layout->setSpacing( 5 );
    layout->setMargin( 5 );
    
    QLabel *l = new QLabel( tr( "Find &Text:" ), this );
    layout->addWidget( l, 0, 0 );
    
    findEdit = new QComboBox( TRUE, this );
    layout->addWidget( findEdit, 1, 0 );
    l->setBuddy( findEdit );
    
    QHBoxLayout *checkboxes = new QHBoxLayout;
    layout->addLayout( checkboxes, 2, 0 );
    
    findCaseSensitive = new QCheckBox( tr( "&Case sensitive" ), this );
    checkboxes->addWidget( findCaseSensitive );

    findWholeWords = new QCheckBox( tr( "&Whole words only" ), this );
    checkboxes->addWidget( findWholeWords );
    
    findButton = new QPushButton( tr( "&Find" ), this );
    findButton->setDefault( TRUE );
    layout->addWidget( findButton, 1, 1 );

    QPushButton *closeButton = new QPushButton( tr( "&Close" ), this );
    connect( closeButton, SIGNAL( clicked() ),
	     this, SLOT( reject() ) );
    connect( closeButton, SIGNAL( clicked() ),
	     this, SIGNAL( closed() ) );
    layout->addWidget( closeButton, 2, 1 );
    
    findEdit->setFocus();
}

void HelpFindDialog::closeEvent( QCloseEvent *e )
{
    QDialog::closeEvent( e );
    emit closed();
}
