#include "resource.h"
#include <qfile.h>
#include <qfileinfo.h>
#include <qapplication.h>

#ifdef Q_OS_WIN32
#include <windows.h>
#endif

/*
   Tries to load the binary resource \a resourceName. If the resource is
   smaller than \a minimumSize, the resource is not loaded and isValid()
   returns false. isValid() returns also false when the loading failed.
 */
ResourceLoader::ResourceLoader( char *resourceName, int minimumSize )
{
#if defined(Q_OS_WIN32)
    valid = true;

    HMODULE hmodule = GetModuleHandle( 0 );
    // we don't need wide character versions
    HRSRC resource = FindResourceA( hmodule, resourceName, MAKEINTRESOURCEA( 10 ) );
    HGLOBAL hglobal = LoadResource( hmodule, resource );
    arSize = SizeofResource( hmodule, resource );
    if ( arSize == 0 ) {
	valid = false;
	return;
    }
    if ( arSize < minimumSize ) {
	valid = false;
	return;
    }
    arData = (char*)LockResource( hglobal );
    if ( arData == 0 ) {
	valid = false;
	return;
    }
    ba.setRawData( arData, arSize );
#elif defined(Q_OS_MAC)
    valid = false;
    arSize = 0;
    arData = 0;
    QFile f;
    QString appDir = qApp->argv()[0];
    int truncpos = appDir.findRev( "/Contents/MacOS/" );
    if (truncpos != -1)
	appDir.truncate( truncpos );
    QString path = appDir + "/Contents/Qt/";
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
    valid = true;
    return;
#endif
}

ResourceLoader::~ResourceLoader()
{
    if ( isValid() )
	ba.resetRawData( arData, arSize );
#if defined(Q_OS_MAC)
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
    HANDLE hExe = BeginUpdateResourceA( applicationName.latin1(), false );
    if ( hExe == 0 ) {
	if ( errorMessage )
	    *errorMessage = QString("Could not load the executable %1.").arg(applicationName);
	return false;
    }
    if ( !UpdateResourceA(hExe,(char*)RT_RCDATA,resourceName,0,data.data(),data.count()) ) {
	EndUpdateResource( hExe, true );
	if ( errorMessage )
	    *errorMessage = QString("Could not update the executable %1.").arg(applicationName);
	return false;
    }
    if ( !EndUpdateResource(hExe,false) ) {
	if ( errorMessage )
	    *errorMessage = QString("Could not update the executable %1.").arg(applicationName);
	return false;
    }

    if ( errorMessage )
	*errorMessage = QString("Updated the executable %1.").arg(applicationName);
    return true;
}
#endif
