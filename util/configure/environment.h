#include <qstring.h>
#include <qt_windows.h>


enum Compiler {
    Unknown = 0,
    BORLAND = 0x01,
    MINGW   = 0x02,
    INTEL   = 0x03,
    MSVC4   = 0x40,
    MSVC5   = 0x50,
    MSVC6   = 0x60,
    NET2002 = 0x70,
    NET2003 = 0x71,
    NET2005 = 0x80
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

