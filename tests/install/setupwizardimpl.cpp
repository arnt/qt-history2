#include "setupwizardimpl.h"

SetupWizardImpl::SetupWizardImpl( QWidget* pParent, const char* pName, bool modal, WFlags f ) : SetupWizard( pParent, pName, modal, f )
{
}

void SetupWizardImpl::clickedPath()
{
    qDebug( "Local function");
}