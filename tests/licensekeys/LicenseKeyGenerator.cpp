// LicenseKeyGenerator.cpp : Implementation of CLicenseKeyGenerator
#include "stdafx.h"
#include "Licensekeys.h"
#include "LicenseKeyGenerator.h"

#include "qstring.h"
#include "qfile.h"
#include "keyinfo.h"

/////////////////////////////////////////////////////////////////////////////
// CLicenseKeyGenerator

static QString BSTR2QString( BSTR src )
{
    QString tmp;

    for( unsigned int i = 0; i < SysStringLen( src ); i++ ) {
	QChar c( src[ i ] );
	tmp += c;
    }
    return tmp;
}

static QString textForDate( const QDate& date )
{
    if ( date.isValid() ) {
	return date.toString( Qt::ISODate );
    } else {
	return QString( "invalid date" );
    }
}

STDMETHODIMP CLicenseKeyGenerator::dummy(BSTR* str)
{
    *str = CComBSTR( "Dummy string" );

    return S_OK;
}

STDMETHODIMP CLicenseKeyGenerator::reset(BSTR keyHome)
{
    QString home = BSTR2QString( keyHome );

    if( !home.length() )
	return E_FAIL;

    for ( uint features = 0; features < (1 << NumFeatures); features++ ) {
	QFile out;
	QString fn;

	fn = home + QString( "\\table.%1" ).arg( features, -2, 16 );

	out.setName( fn );
	if ( !out.open(IO_WriteOnly) )
	    return E_FAIL;

	for ( uint bits = 0; bits < (1 << NumRandomBits); bits++ ) {
	    QString k = keyForFeatures( features, bits ) + QChar( '\n' );
	    out.writeBlock( k.latin1(), k.length() );

#if 1
	    if ( featuresForKey(k) != features )
		return E_FAIL;
	    if ( (features & ~(Feature_US | Feature_Enterprise | Feature_Unix)) == 0 )
		if ( featuresForKeyOnUnix(k) != features )
		    return E_FAIL;
#endif
	}

	out.close();

	fn = home + QString( "\\next.%1" ).arg( features, -2, 16 );
	out.setName( fn );
	if ( !out.open(IO_WriteOnly) )
	    return E_FAIL;
	out.writeBlock( "1\n", 2 ); // skip first key
	out.close();
    }
    return S_OK;
}

STDMETHODIMP CLicenseKeyGenerator::newKey(BSTR keyHome,BSTR expiryDate, int us, int enterprise, int windows, int unix, int embedded, int mac, int extra1, int extra2, BSTR *key)
{
    uint features = 0;
    QString home = BSTR2QString( keyHome );

    QDate expDate = QDate::fromString( BSTR2QString( expiryDate ), Qt::ISODate );
    if ( !expDate.isValid() )
	return E_FAIL;

    if( us )
        features |= Feature_US;
    if( enterprise )
        features |= Feature_Enterprise;
    if( windows )
        features |= Feature_Windows;
    if( unix )
        features |= Feature_Unix;
    if( mac )
        features |= Feature_Mac;
    if( embedded )
        features |= Feature_Embedded;
    if( extra1 )
        features |= Feature_Extra1;
    if( extra2 )
        features |= Feature_Extra2;

    QFile in;
    QFile out;
    QString fn;
    QString keyTxt;
    char block[10];

    fn = home + QString( "\\next.%1" ).arg( features, -2, 16 );
    in.setName( fn );
    if ( !in.open(IO_ReadOnly) )
	return E_FAIL;
    int ent = QString( in.readAll() ).toInt();
    in.close();

    fn = home + QString( "\\table.%1" ).arg( features, -2, 16 );
    in.setName( fn );
    if ( !in.open(IO_ReadOnly) )
	return E_FAIL;
    in.at( ent * 10 );
    in.readBlock( block, 9 );
    block[9] = '\0';
    in.close();

    ent++;
    if ( ent == (1 << NumRandomBits) )
	ent = 1; // skip first entry

    fn = home + QString( "\\next.%1" ).arg( features, -2, 16 );
    out.setName( fn );
    if ( !out.open(IO_WriteOnly) )
	return E_FAIL;
    QString s = QString::number( ent ) + QChar( '\n' );
    out.writeBlock( s, s.length() );
    out.close();

    if ( strlen(block) == 9 && block[4] == '-' ) {
	keyTxt = block + encodedExpiryDate(expDate);
	*key = ::SysAllocString( (TCHAR*)qt_winTchar( keyTxt, TRUE ) );
    }
    return S_OK;
}

STDMETHODIMP CLicenseKeyGenerator::decodeExpiryDate(BSTR key, BSTR *expDate)
{
    QString keyTxt = BSTR2QString( key );

    if ( featuresForKey( keyTxt ) != 0 )
	*expDate = ::SysAllocString( (WCHAR*)qt_winTchar( textForDate( decodedExpiryDate( keyTxt.mid( 9 ) ) ), TRUE ) );

    return S_OK;
}

STDMETHODIMP CLicenseKeyGenerator::unlocksUS(BSTR key, int *valid)
{
    QString keyTxt = BSTR2QString( key );

    *valid = ( featuresForKey( keyTxt ) & Feature_US );

    return S_OK;
}

STDMETHODIMP CLicenseKeyGenerator::unlocksEnterprise(BSTR key, int *valid)
{
    QString keyTxt = BSTR2QString( key );

    *valid = ( featuresForKey( keyTxt ) & Feature_Enterprise );

    return S_OK;
}

STDMETHODIMP CLicenseKeyGenerator::unlocksWindows(BSTR key, int *valid)
{
    QString keyTxt = BSTR2QString( key );

    *valid = ( featuresForKey( keyTxt ) & Feature_Windows );

    return S_OK;
}

STDMETHODIMP CLicenseKeyGenerator::unlocksUnix(BSTR key, int *valid)
{
    QString keyTxt = BSTR2QString( key );

    *valid = ( featuresForKey( keyTxt ) & Feature_Unix );

    return S_OK;
}

STDMETHODIMP CLicenseKeyGenerator::unlocksEmbedded(BSTR key, int *valid)
{
    QString keyTxt = BSTR2QString( key );

    *valid = ( featuresForKey( keyTxt ) & Feature_Embedded );

    return S_OK;
}

STDMETHODIMP CLicenseKeyGenerator::unlocksMac(BSTR key, int *valid)
{
    QString keyTxt = BSTR2QString( key );

    *valid = ( featuresForKey( keyTxt ) & Feature_Mac );

    return S_OK;
}

STDMETHODIMP CLicenseKeyGenerator::unlocksExtra1(BSTR key, int *valid)
{
    QString keyTxt = BSTR2QString( key );

    *valid = ( featuresForKey( keyTxt ) & Feature_Extra1 );

    return S_OK;
}

STDMETHODIMP CLicenseKeyGenerator::unlocksExtra2(BSTR key, int *valid)
{
    QString keyTxt = BSTR2QString( key );

    *valid = ( featuresForKey( keyTxt ) & Feature_Extra2 );

    return S_OK;
}

