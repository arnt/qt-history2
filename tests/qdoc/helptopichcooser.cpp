#include "helptopichcooser.h"
#include <qlayout.h>
#include <qlabel.h>
#include <qlistbox.h>
#include <qpushbutton.h>

HelpTopicChooser::HelpTopicChooser( QWidget *parent, const QStringList &lnkNames,
				    const QStringList &lnks, const QString &title )
    : QDialog( parent, "", TRUE ), links( lnks ), linkNames( lnkNames )
{
    QVBoxLayout *layout = new QVBoxLayout( this );
    layout->setMargin( 5 );
    layout->setSpacing( 5 );
    
    QLabel *l = new QLabel( tr( "Choose a topic for <b>%1</b>" ).arg( title ), this );
    layout->addWidget( l );
    
    linkList = new QListBox( this );
    linkList->insertStringList( linkNames );
    layout->addWidget( linkList );
    linkList->setCurrentItem( 0 );
    
    QHBoxLayout *buttonLayout = new QHBoxLayout( layout );
    buttonLayout->setSpacing( 5 );
    buttonLayout->setAlignment( AlignRight );
    buttonLayout->addStretch();
    QPushButton *pb = new QPushButton( tr( "&Display" ), this );
    pb->setDefault( TRUE );
    connect( pb, SIGNAL( clicked() ),
	     this, SLOT( accept() ) );
    buttonLayout->addWidget( pb );
    pb->setFixedSize( pb->sizeHint() );
    pb = new QPushButton( tr( "&Close" ), this );
    buttonLayout->addWidget( pb );
    pb->setFixedSize( pb->sizeHint() );
    connect( pb, SIGNAL( clicked() ),
	     this, SLOT( reject() ) );
    connect( linkList, SIGNAL( doubleClicked( QListBoxItem * ) ),
	     this, SLOT( accept() ) );
    connect( linkList, SIGNAL( returnPressed( QListBoxItem * ) ),
	     this, SLOT( accept() ) );
    
    resize( sizeHint().width() * 2, sizeHint().height() );
}
    
QString HelpTopicChooser::link() const
{
    if ( linkList->currentItem() == -1 )
	return QString::null;
    QString s = linkList->currentText();
    if ( s.isEmpty() )
	return s;
    int i = linkNames.findIndex( s );
    return links[ i ];
}

QString HelpTopicChooser::getLink( QWidget *parent, const QStringList &linkNames, 
				   const QStringList &links, const QString &title )
{
    HelpTopicChooser *dlg = new HelpTopicChooser( parent, linkNames, links, title );
    QString lnk;
    if ( dlg->exec() == QDialog::Accepted )
	lnk = dlg->link();
    delete dlg;
    return lnk;
}

