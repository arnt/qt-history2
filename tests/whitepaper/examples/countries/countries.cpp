/*
  countries.cpp
*/

#include <qapplication.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlistbox.h>
#include <qpushbutton.h>

class MyDialog : public QDialog
{
public:
    MyDialog();
};

MyDialog::MyDialog()
{
    // quote
    setCaption( "International Trader" );

    QVBoxLayout *buttonBox = new QVBoxLayout( 6 );
    buttonBox->addWidget( new QPushButton("OK", this) );
    buttonBox->addWidget( new QPushButton("Cancel", this) );
    buttonBox->addStretch( 1 );
    buttonBox->addWidget( new QPushButton("Help", this) );

    QListBox *countryList = new QListBox( this );
    countryList->insertItem( "Canada" );
    /* ... */
    countryList->insertItem( "United States of America" );

    QHBoxLayout *middleBox = new QHBoxLayout( 11 );
    middleBox->addWidget( countryList );
    middleBox->addLayout( buttonBox );

    QVBoxLayout *topLevelBox = new QVBoxLayout( this, 6, 11 ); 
    topLevelBox->addWidget( new QLabel("Select a country", this) );
    topLevelBox->addLayout( middleBox );
    // endquote

    countryList->insertItem( "France" );
    countryList->insertItem( "Germany" );
    countryList->insertItem( "Italy" );
    countryList->insertItem( "Japan" );
    countryList->insertItem( "Russia" );
    countryList->insertItem( "United Kingdom" );
    countryList->sort();

    show();

    countryList->setSelected( 6, TRUE );
    countryList->setBottomItem( 7 );
}

int main( int argc, char **argv )
{
    QApplication app( argc, argv );
    MyDialog d;
    app.setMainWidget( &d );
    return app.exec();
}
