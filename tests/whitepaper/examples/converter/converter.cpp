/*
  converter.cpp
*/

#include <qapplication.h>
#include <qcstring.h>
#include <qlayout.h>
#include <qpushbutton.h>
#include <qstringlist.h>

#include "converter.h"

Converter::Converter( QWidget *parent, const char *name )
    : QDialog( parent, name ), socket( 0 )
{
    QVBoxLayout *vbox = new QVBoxLayout( this, 11, 11 );

    QGridLayout *grid = new QGridLayout( vbox, 2, 2, 6 );

    sourceAmount = new QLineEdit( "100", this );
    sourceCurrency = create_currency_combobox( "EUR" );
    targetAmount = new QLabel( this );
    targetAmount->setFrameStyle( QFrame::WinPanel | QFrame::Sunken );
    targetCurrency = create_currency_combobox( "USD" );

    QPushButton *convertButton = new QPushButton( "Convert", this );

    QHBoxLayout *hbox = new QHBoxLayout( vbox, 6 );
    hbox->addStretch( 1 );
    hbox->addWidget( convertButton );
    hbox->addStretch( 1 );

    grid->addWidget( sourceAmount, 0, 0 );
    grid->addWidget( sourceCurrency, 0, 1 );
    grid->addWidget( targetAmount, 1, 0 );
    grid->addWidget( targetCurrency, 1, 1 );

    socket = new QSocket( this );
    connect( socket, SIGNAL(readyRead()),
	     this, SLOT(updateTargetAmount()) );

    connect( sourceAmount, SIGNAL(textChanged(const QString&)),
	     targetAmount, SLOT(clear()) );
    connect( convertButton, SIGNAL(clicked()), this, SLOT(convert()) );
}

void Converter::convert()
{
    QString command = "CONV " + sourceAmount->text() + " " +
		      sourceCurrency->currentText() + " " +
		      targetCurrency->currentText() + "\r\n";
    socket->connectToHost( "ccp.banca-monica.nu", 123 );
    socket->writeBlock( command.latin1(), command.length() );
}

void Converter::updateTargetAmount()
{
    if ( socket->canReadLine() ) {
	targetAmount->setText( socket->readLine() );
	socket->close();
    }
}

QComboBox *Converter::create_currency_combobox( const char *initial )
{
    const char currencies[] = "CAD DEM EUR FRF GBP ITL JPY RUR USD";
    QComboBox *currency = new QComboBox( this );
    currency->insertStringList( QStringList::split(' ', currencies) );
    currency->setCurrentText( initial );
    return currency;
}
