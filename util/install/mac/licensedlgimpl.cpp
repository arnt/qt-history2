#include "licensedlgimpl.h"
#include <qfile.h>
#include <qtextview.h>

LicenseDialogImpl::LicenseDialogImpl( QWidget *parent )
    : LicenseDialog( parent )
{
}

bool LicenseDialogImpl::showLicense( bool licenseUs )
{
    QFile f;
    if (licenseUs)
	f.setName( "LICENSE-US" );
    else
	f.setName( "LICENSE" );
    if ( !f.open( IO_ReadOnly ) )
	return false;
    
    QTextStream ts( &f );
    licenseText->setText( ts.read() );
    return true;
}


