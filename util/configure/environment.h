#include <qstring.h>
#include <qt_windows.h>


enum Compiler {
    CC_UNKNOWN = 0,
    CC_BORLAND = 0x01,
    CC_MINGW   = 0x02,
    CC_INTEL   = 0x03,
    CC_MSVC4   = 0x40,
    CC_MSVC5   = 0x50,
    CC_MSVC6   = 0x60,
    CC_NET2002 = 0x70,
    CC_NET2003 = 0x71,
    CC_NET2005 = 0x80
};

struct CompilerInfo;
class Environment
{
public:
    static Compiler detectCompiler();
    static QString detectQMakeSpec();
    static bool detectExecutable(const QString &executable);

    static int execute(QStringList arguments, const QStringList &additionalEnv, const QStringList &removeEnv);

private:
    static Compiler detectedCompiler;

    static CompilerInfo *compilerInfo(Compiler compiler);
    static QString keyPath(const QString &rKey);
    static QString keyName(const QString &rKey);
    static QString readRegistryKey(HKEY parentHandle, const QString &rSubkey);
};

