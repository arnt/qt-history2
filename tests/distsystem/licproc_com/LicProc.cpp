// LicProc.cpp : Implementation of LicProc
#include "stdafx.h"
#include "Licproc_com.h"
#include "LicProc.h"

#include "qstring.h"

#include <io.h>

/////////////////////////////////////////////////////////////////////////////
// LicProc


static QString BSTR2QString( BSTR src )
{
    QString tmp;

    for( unsigned int i = 0; i < SysStringLen( src ); i++ ) {
	QChar c( src[ i ] );
	tmp += c;
    }
    return tmp;
}

static bool checkCompanyId( QString companyId )
{
    if( ( companyId == "nor" ) || ( companyId == "usa" ) )
	return true;

    return false;
}

STDMETHODIMP LicProc::dummy(DWORD bar)
{
    DWORD foo = bar;
    return S_OK;
}

STDMETHODIMP LicProc::publishLicense(DWORD licenseId, BSTR custId, BSTR licensee, BSTR licenseeEmail, BSTR login, BSTR password, BSTR expiryDate, BSTR companyId)
{
    return E_NOTIMPL;
}

STDMETHODIMP LicProc::publishLine(DWORD licenseId, BSTR itemId, DWORD usLicense, BSTR companyId)
{
    return E_NOTIMPL;
}

STDMETHODIMP LicProc::setServer(BSTR srv, BSTR user, BSTR password, BSTR db)
{
    srvName = BSTR2QString( srv );

    return S_OK;
}

STDMETHODIMP LicProc::updateDb(DWORD licenseId, BSTR versionTag, BSTR companyId)
{
    QString tag = BSTR2QString( versionTag ) + "\n";

    if( !checkCompanyId( BSTR2QString( companyId ) ) )
	return S_OK;

    int sock = socket( AF_INET, SOCK_STREAM, 0 );
    if( !sock )
	return E_FAIL;

    struct sockaddr_in srvAddr;

    memset( &srvAddr, 0, sizeof( srvAddr ) );
    struct hostent* he = gethostbyname( srvName.latin1() );

    srvAddr.sin_family = AF_INET;
    srvAddr.sin_addr = *(in_addr*)( he->h_addr_list[ 0 ] );
    srvAddr.sin_port = htons( 801 );
    if( !connect( sock, (struct sockaddr*)&srvAddr, sizeof( srvAddr ) ) ) {
	char buffer( 0 );
	QString str;
	bool sawGreeting( false );

	while( 1 ) {
	    while( buffer != '\n' ) {
		if( recv( sock, &buffer, sizeof( buffer ), 0 ) == 1) {
		    str += buffer;
		}
		else {
//		    close( sock );
		    return S_OK;
		}
	    }
	    if( !sawGreeting ) {
		sawGreeting = true;
		send( sock, tag.latin1(), tag.length(), 0 );
	    }
	    buffer = 0;
	    str = "";
	}
	close( sock );
    }
    
    return S_OK;
}

/*
** For some reason, this function is required for release builds,
** but not debug...
*/
int main(int argc, char** argv )
{
    return 0;
}

STDMETHODIMP LicProc::publishVersionTag(BSTR tag, BSTR versionString, BSTR subDir, BSTR companyId)
{
    return E_NOTIMPL;
}

STDMETHODIMP LicProc::publishFilemap(BSTR tag, BSTR itemId, BSTR fileName, BSTR fileDesc, BSTR companyId)
{
    return E_NOTIMPL;
}

STDMETHODIMP LicProc::clearVersionTags(BSTR companyId)
{
    return E_NOTIMPL;
}

STDMETHODIMP LicProc::clearFilemap(BSTR companyId)
{
    return E_NOTIMPL;
}

STDMETHODIMP LicProc::deleteVersionTag(BSTR tag, BSTR companyId)
{
    return E_NOTIMPL;
}

STDMETHODIMP LicProc::deleteFilemap(BSTR tag, BSTR itemId, BSTR fileName, BSTR companyId)
{
    return E_NOTIMPL;
}

STDMETHODIMP LicProc::deleteLicense(BSTR licenseId, BSTR companyId)
{
    return E_NOTIMPL;
}
