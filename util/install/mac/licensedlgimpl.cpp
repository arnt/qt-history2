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
    QString path = "qt-mac-commercial-3.0.0.app/Contents/Qt/LICENSE";
    if (licenseUs)
	path.append( "-US" );
    f.setName( path );
    if ( !f.open( IO_ReadOnly ) )
	return false;
    
    QTextStream ts( &f );
    licenseText->setText( ts.read() );
    return true;
}


