#include "pages.h"

BuildPageImpl::BuildPageImpl( QWidget* parent, const char* name, WFlags fl )
    : BuildPage( parent, name, fl )
{
}

ConfigPageImpl::ConfigPageImpl( QWidget* parent, const char* name, WFlags fl )
    : ConfigPage( parent, name, fl )
{
}

FinishPageImpl::FinishPageImpl( QWidget* parent, const char* name, WFlags fl )
    : FinishPage( parent, name, fl )
{
}

FoldersPageImpl::FoldersPageImpl( QWidget* parent, const char* name, WFlags fl )
    : FoldersPage( parent, name, fl )
{
}

LicenseAgreementPageImpl::LicenseAgreementPageImpl( QWidget* parent, const char* name, WFlags fl )
    : LicenseAgreementPage( parent, name, fl )
{
}

LicensePageImpl::LicensePageImpl( QWidget* parent, const char* name, WFlags fl )
    : LicensePage( parent, name, fl )
{
}

OptionsPageImpl::OptionsPageImpl( QWidget* parent, const char* name, WFlags fl )
    : OptionsPage( parent, name, fl )
{
}

ProgressPageImpl::ProgressPageImpl( QWidget* parent, const char* name, WFlags fl )
    : ProgressPage( parent, name, fl )
{
}
