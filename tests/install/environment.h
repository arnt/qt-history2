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

    enum {
	LocalEnv = 1,
	PersistentEnv = 2
    };

    enum {
	CurrentUser = 0,
	LocalMachine = 1
    };
};

