#ifndef ENVIRONMENT_H
#define ENVIRONMENT_H

#include <qstring.h>

class QEnvironment
{
public:
    static QString getEnv( const QString &varName, int envBlock = LocalEnv );
    static void putEnv( const QString &varName, const QString &varValue, int envBlock = LocalEnv );
    static void removeEnv( const QString &varName, int envBlock = LocalEnv );
#if defined(Q_OS_WIN32)
    static QString getRegistryString( const QString &keyName, const QString &valueName, int scope = CurrentUser );
    static void recordUninstall( const QString &displayName, const QString &cmdString );
    static void removeUninstall( const QString &displayName );
#endif
    static QString getTempPath();
    static QString getLastError();
    static QString getFSFileName( const QString& fileName );

    enum {
	LocalEnv = 1,
	PersistentEnv = 2,
	GlobalEnv = 4
    };

    enum {
	CurrentUser = 0,
	LocalMachine = 1
    };
};

#endif
