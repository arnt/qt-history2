#include "setdataform.h"

#include <qcolordialog.h>
#include <qlayout.h>
#include <qpixmap.h>
#include <qpushbutton.h>
#include <qtable.h>

// TODO NUMBER_FORMAT and DECIMAL_PLACES should be Options.


SetDataForm::SetDataForm( ElementVector *elements,
			  QWidget* parent,  const char* name,
			  bool modal, WFlags fl )
    : QDialog( parent, name, modal, fl )

{
    pelements = elements;

    setName( "SetDataForm" );
    setCaption( "Chart -- Set Data" );
    resize( 480, 400 );

    tableButtonBox = new QVBoxLayout( this, 11, 6, "table button box layout" );
    table = new QTable( this, "data table" );
    table->setNumCols( 4 );
    table->setNumRows( MAX_ELEMENTS );
    table->setColumnReadOnly( 1, true );
    table->setColumnReadOnly( 3, true );
    table->setColumnWidth( 0, 80 );
    table->setColumnWidth( 1, 60 ); // Columns 1 and 3 must be equal
    table->setColumnWidth( 2, 200 );
    table->setColumnWidth( 3, 60 );
    QHeader *th = table->horizontalHeader();
    th->setLabel( 0, "Value" );
    th->setLabel( 1, "Colour" );
    th->setLabel( 2, "Label" );
    th->setLabel( 3, "Colour" );
    tableButtonBox->addWidget( table );

    buttonBox = new QHBoxLayout( 0, 0, 6, "button box layout" );
    colourPushButton = new QPushButton( this, "colour button" );
    colourPushButton->setText( "&Colour..." );
    colourPushButton->setEnabled( false );
    buttonBox->addWidget( colourPushButton );

    QSpacerItem *spacer = new QSpacerItem( 0, 0, QSizePolicy::Expanding,
						 QSizePolicy::Minimum );
    buttonBox->addItem( spacer );

    okPushButton = new QPushButton( this, "ok button" );
    okPushButton->setText( "OK" );
    okPushButton->setDefault( TRUE );
    buttonBox->addWidget( okPushButton );

    cancelPushButton = new QPushButton( this, "cancel button" );
    cancelPushButton->setText( "Cancel" );
    cancelPushButton->setAccel( Key_Escape );
    buttonBox->addWidget( cancelPushButton );

    tableButtonBox->addLayout( buttonBox );

    connect( table, SIGNAL( clicked(int,int,int,const QPoint&) ),
	     this, SLOT( setColour(int,int) ) );
    connect( table, SIGNAL( currentChanged(int,int) ),
	     this, SLOT( currentChanged(int,int) ) );
    connect( table, SIGNAL( valueChanged(int,int) ),
	     this, SLOT( valueChanged(int,int) ) );
    connect( colourPushButton, SIGNAL( clicked() ), this, SLOT( setColour() ) );
    connect( okPushButton, SIGNAL( clicked() ), this, SLOT( accept() ) );
    connect( cancelPushButton, SIGNAL( clicked() ), this, SLOT( reject() ) );

    QRect rect = table->cellRect( 0, 1 );
    QPixmap pix( rect.width(), rect.height() );

    for ( int i = 0; i < MAX_ELEMENTS; ++i ) {
	Element element = (*pelements)[i];

	if ( element.isValid() )
	    table->setText(
		i, 0,
		QString( "%1" ).arg( element.getValue(), 0,
				     NUMBER_FORMAT, DECIMAL_PLACES ) );

	QColor colour = element.getValueColour();
	pix.fill( colour );
	table->setPixmap( i, 1, pix );
	table->setText( i, 1, colour.name() );

	table->setText( i, 2, element.getLabel() );

	colour = element.getLabelColour();
	pix.fill( colour );
	table->setPixmap( i, 3, pix );
	table->setText( i, 3, colour.name() );
    }

}


void SetDataForm::currentChanged( int, int col )
{
    colourPushButton->setEnabled( col == 1 || col == 3 );
}


void SetDataForm::valueChanged( int row, int col )
{
    if ( col == 0 ) {
	bool ok;
	double d = table->text( row, col ).toDouble( &ok );
	if ( ok && d > EPSILON )
	    table->setText(
		row, col, QString( "%1" ).arg(
			    d, 0, NUMBER_FORMAT, DECIMAL_PLACES ) );
	else
	    ; // TODO Handle error
    }
}


void SetDataForm::setColour()
{
    setColour( table->currentRow(), table->currentColumn() );
    table->setFocus();
}


void SetDataForm::setColour( int row, int col )
{
    QColor colour = QColorDialog::getColor(
			QColor( table->text( row, col ) ),
			this, "colour dialog" );
    if ( colour.isValid() ) {
	QPixmap pix = table->pixmap( row, col );
	pix.fill( colour );
	table->setPixmap( row, col, pix );
	table->setText( row, col, colour.name() );
    }
}


void SetDataForm::accept()
{
    bool ok;
    for ( int i = 0; i < MAX_ELEMENTS; ++i ) {
	Element &element = (*pelements)[i];
	double d = table->text( i, 0 ).toDouble( &ok );
	if ( ok )
	    element.setValue( d );
	else
	    element.setValue( INVALID );
	element.setValueColour( QColor( table->text( i, 1 ) ) );
	element.setLabel( table->text( i, 2 ) );
	element.setLabelColour( QColor( table->text( i, 3 ) ) );
    }

    QDialog::accept();
}
