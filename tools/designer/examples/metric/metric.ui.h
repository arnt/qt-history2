/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions respectively slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

#include <qvalidator.h>

void dialog::init()
{
    editInput->setValidator( new QDoubleValidator( editInput ) );
}

void dialog::convert()
{
    enum MetricUnits {
	Kilometers,
	Meters,
	Centimeters,
	Millimeters
    };
    enum OldUnits {
	Miles,
	Yards,
	Feet,
	Inches
    };
    
    // Retrieve the input
    double input = editInput->text().toDouble();
    
    // internally convert the input to millimeters
    switch ( comboFrom->currentItem() ) {
    case Kilometers:
	input *= 1000000;
	break;
    case Meters:
	input *= 1000;
	break;
    case Centimeters:
	input *= 10;
	break;
    }
    
    //convert to inches
    double result = input * 0.0393701;
    
    switch ( comboTo->currentItem() ) {
    case Miles:
	result /= 63360;
	break;
    case Yards:
	result /= 36;
	break;
    case Feet:
	result /= 12;
	break;
    }
    
    // set the result
    editResult->setText( QString::number( result ) );
}
