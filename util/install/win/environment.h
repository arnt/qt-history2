#include <qstring.h>

class QEnvironment
{
public:
    static QString getEnv( QString varName, int envBlock = LocalEnv );
    static void putEnv( QString varName, QString varValue, int envBlock = LocalEnv );
    static QString getRegistryString( QString keyName, QString valueName, int scope = CurrentUser );
    static bool recordUninstall( QString displayName, QString cmdString );
    static bool removeUninstall( QString displayName );
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

