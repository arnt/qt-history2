#include <qstring.h>

class QEnvironment
{
public:
    static QString getEnv( QString varName, int envBlock = LocalEnv );
    static void putEnv( QString varName, QString varValue, int envBlock = LocalEnv );
    static QString getRegistryString( QString keyName, QString valueName, int scope = CurrentUser );
    static QString getTempPath();

    enum {
	LocalEnv = 1,
	DefaultEnv = 2,
	SystemEnv = 4
    };

    enum {
	CurrentUser = 0,
	LocalMachine = 1
    };
};

