#include "resource.h"
#include <qfile.h>
#include <qfileinfo.h>

#ifdef Q_OS_WIN32
#include <windows.h>
#endif

/*
   Tries to load the binary resource \a resourceName. If the resource is
   smaller than \a minimumSize, the resource is not loaded and isValid()
   returns FALSE. isValid() returns also FALSE when the loading failed.
 */
ResourceLoader::ResourceLoader( char *resourceName, int minimumSize )
{
#if defined(Q_OS_WIN32)
    valid = TRUE;

    HMODULE hmodule = GetModuleHandle( 0 );
    // we don't need wide character versions
    HRSRC resource = FindResourceA( hmodule, resourceName, MAKEINTRESOURCEA( 10 ) );
    HGLOBAL hglobal = LoadResource( hmodule, resource );
    arSize = SizeofResource( hmodule, resource );
    if ( arSize == 0 ) {
	valid = FALSE;
	return;
    }
    if ( arSize < minimumSize ) {
	valid = FALSE;
	return;
    }
    arData = (char*)LockResource( hglobal );
    if ( arData == 0 ) {
	valid = FALSE;
	return;
    }
    ba.setRawData( arData, arSize );
#elif defined(Q_OS_MACX)
    valid = FALSE;
    arSize = 0;
    arData = 0;
    QFile f;
    QString path = "install.app/Contents/Qt/";
    path += resourceName;
    f.setName( path );
    if (!f.open( IO_ReadOnly ))
	return;
    QFileInfo fi(f);
    arSize = fi.size();
    arData = new char[arSize];
    if (f.readBlock( arData, arSize ) != arSize)
    {
	delete[] arData;
	return;
    }
    ba.setRawData( arData, arSize );
    valid = TRUE;
    return;
#endif
}

ResourceLoader::~ResourceLoader()
{
    if ( isValid() )
	ba.resetRawData( arData, arSize );
#if defined(Q_OS_MACX)
    delete[] arData;
#endif
}

bool ResourceLoader::isValid() const
{
    return valid;
}

QByteArray ResourceLoader::data()
{
    return ba;
}


#if defined(Q_OS_WIN32)
ResourceSaver::ResourceSaver( const QString& appName )
    : applicationName(appName)
{
}

ResourceSaver::~ResourceSaver()
{
}

bool ResourceSaver::setData( char *resourceName, const QByteArray &data, QString *errorMessage )
{
    // we don't need wide character versions
    HANDLE hExe = BeginUpdateResourceA( applicationName.latin1(), FALSE );
    if ( hExe == 0 ) {
	if ( errorMessage )
	    *errorMessage = QString("Could not load the executable %1.").arg(applicationName);
	return FALSE;
    }
    if ( !UpdateResourceA(hExe,RT_RCDATA,resourceName,0,data.data(),data.count()) ) {
	EndUpdateResource( hExe, TRUE );
	if ( errorMessage )
	    *errorMessage = QString("Could not update the executable %1.").arg(applicationName);
	return FALSE;
    }
    if ( !EndUpdateResource(hExe,FALSE) ) {
	if ( errorMessage )
	    *errorMessage = QString("Could not update the executable %1.").arg(applicationName);
	return FALSE;
    }

    if ( errorMessage )
	*errorMessage = QString("Updated the executable %1.").arg(applicationName);
    return TRUE;
}
#endif
