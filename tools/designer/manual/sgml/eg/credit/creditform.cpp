#include <qspinbox.h>
#include "creditform.h"

CreditForm::CreditForm( QWidget* parent, const char* name, 
			bool modal, WFlags fl )
    : CreditFormBase( parent, name, modal, fl ) { /* NOOP */ }

CreditForm::~CreditForm() { /* NOOP */ }

void CreditForm::special( bool on ) 
{
    amountSpinBox->setEnabled( on ); 
}
