#include "environment.h"

#include <process.h>
#include <iostream>

using namespace std;

#ifdef Q_OS_WIN32
#include <qt_windows.h>
#endif


struct CompilerInfo{
    Compiler compiler;
    const char *compilerStr;
    const char *regKey;
    const char *executable;
} compiler_info[] = {
    // The compilers here are sorted in a reversed-preferred order
    {BORLAND, "Borland C++",                                                    0, "bcc32.exe"},
    {MINGW,   "MinGW (Minimalist GNU for Windows)",                             0, "mingw32-gcc.exe"},
    {INTEL,   "Intel(R) C++ Compiler for 32-bit applications",                  0, "icl.exe"}, // xilink.exe, xilink5.exe, xilink6.exe, xilib.exe
    {MSVC6,   "Microsoft (R) 32-bit C/C++ Optimizing CompilerMSVC 6.x",         "Software\\Microsoft\\VisualStudio\\6.0\\Setup\\Microsoft Visual C++\\ProductDir", "cl.exe"}, // link.exe, lib.exe
    {NET2002, "Microsoft (R) 32-bit C/C++ Optimizing Compiler.NET 2002 (7.0)",  "Software\\Microsoft\\VisualStudio\\7.0\\Setup\\VC\\ProductDir", "cl.exe"}, // link.exe, lib.exe
    {NET2003, "Microsoft (R) 32-bit C/C++ Optimizing Compiler.NET 2003 (7.1)",  "Software\\Microsoft\\VisualStudio\\7.1\\Setup\\VC\\ProductDir", "cl.exe"}, // link.exe, lib.exe
    {NET2005, "Microsoft (R) 32-bit C/C++ Optimizing Compiler.NET 2005 (8.0)",  "Software\\Microsoft\\VisualStudio\\8.0\\Setup\\VC\\ProductDir", "cl.exe"}, // link.exe, lib.exe
    {Unknown, "Unknown", 0, 0},
};


// Initialize static variables
Compiler Environment::detectedCompiler = Unknown;

/*!
    Returns the pointer to the CompilerInfo for a \a compiler.
*/
CompilerInfo *Environment::compilerInfo(Compiler compiler)
{
    int i = 0;
    while(compiler_info[i].compiler != compiler && compiler_info[i].compiler != Unknown)
        ++i;
    return &(compiler_info[i]);
}

/*!
    Returns the path part of a registry key.
    Ei.
        For a key
            "Software\\Microsoft\\VisualStudio\\8.0\\Setup\\VC\\ProductDir"
        it returns
            "Software\\Microsoft\\VisualStudio\\8.0\\Setup\\VC\\"
*/
QString Environment::keyPath(const QString &rKey)
{
    int idx = rKey.lastIndexOf(QLatin1Char('\\'));
    if (idx == -1)
        return QString();
    return rKey.left(idx + 1);
}

/*!
    Returns the name part of a registry key.
    Ei.
        For a key
            "Software\\Microsoft\\VisualStudio\\8.0\\Setup\\VC\\ProductDir"
        it returns
            "ProductDir"
*/
QString Environment::keyName(const QString &rKey)
{
    int idx = rKey.lastIndexOf(QLatin1Char('\\'));
    if (idx == -1)
        return rKey;

    QString res(rKey.mid(idx + 1));
    if (res == "Default" || res == ".")
        res = "";
    return res;
}

/*!
    Returns a registry keys value in string form.
    If the registry key does not exist, or cannot be accessed, a
    QString() is returned.
*/
QString Environment::readRegistryKey(HKEY parentHandle, const QString &rSubkey)
{
#ifndef Q_OS_WIN32
    return QString();
#else
    QString rSubkeyName = keyName(rSubkey);
    QString rSubkeyPath = keyPath(rSubkey);

    HKEY handle = 0;
    LONG res;
    QT_WA( {
        res = RegOpenKeyExW(parentHandle, (WCHAR*)rSubkeyPath.utf16(),
                            0, KEY_READ, &handle);
    } , {
        res = RegOpenKeyExA(parentHandle, rSubkeyPath.toLocal8Bit(),
                            0, KEY_READ, &handle);
    } );

    if (res != ERROR_SUCCESS)
        return QString();

    // get the size and type of the value
    DWORD dataType;
    DWORD dataSize;
    QT_WA( {
        res = RegQueryValueExW(handle, (WCHAR*)rSubkeyName.utf16(), 0, &dataType, 0, &dataSize);
    }, {
        res = RegQueryValueExA(handle, rSubkeyName.toLocal8Bit(), 0, &dataType, 0, &dataSize);
    } );
    if (res != ERROR_SUCCESS) {
        RegCloseKey(handle);
        return QString();
    }

    // get the value
    QByteArray data(dataSize, 0);
    QT_WA( {
        res = RegQueryValueExW(handle, (WCHAR*)rSubkeyName.utf16(), 0, 0,
                               reinterpret_cast<unsigned char*>(data.data()), &dataSize);
    }, {
        res = RegQueryValueExA(handle, rSubkeyName.toLocal8Bit(), 0, 0,
                               reinterpret_cast<unsigned char*>(data.data()), &dataSize);
    } );
    if (res != ERROR_SUCCESS) {
        RegCloseKey(handle);
        return QString();
    }

    QString result;
    switch (dataType) {
        case REG_EXPAND_SZ:
        case REG_SZ: {
            QT_WA( {
                result = QString::fromUtf16(((const ushort*)data.constData()));
            }, {
                result = QString::fromLatin1(data.constData());
            } );
            break;
        }

        case REG_MULTI_SZ: {
            QStringList l;
            int i = 0;
            for (;;) {
                QString s;
                QT_WA( {
                    s = QString::fromUtf16((const ushort*)data.constData() + i);
                }, {
                    s = QString::fromLatin1(data.constData() + i);
                } );
                i += s.length() + 1;

                if (s.isEmpty())
                    break;
                l.append(s);
            }
	    result = l.join(", ");
            break;
        }

        case REG_NONE:
        case REG_BINARY: {
            QT_WA( {
                result = QString::fromUtf16((const ushort*)data.constData(), data.size()/2);
            }, {
                result = QString::fromLatin1(data.constData(), data.size());
            } );
            break;
        }

        case REG_DWORD_BIG_ENDIAN:
        case REG_DWORD: {
            Q_ASSERT(data.size() == sizeof(int));
            int i;
            memcpy((char*)&i, data.constData(), sizeof(int));
	    result = QString::number(i);
            break;
        }

        default:
            qWarning("QSettings: unknown data %d type in windows registry", dataType);
            break;
    }

    RegCloseKey(handle);
    return result;
#endif
}

/*!
    Returns the qmakespec for the compiler detected on the system.
*/
QString Environment::detectQMakeSpec()
{
    QString spec;
    switch (detectCompiler()) {
    case NET2005:
        spec = "win32-msvc2005";
        break;
    case NET2002:
    case NET2003:
        spec = "win32-msvc.net";
        break;
    case MSVC4:
    case MSVC5:
    case MSVC6:
        spec = "win32-msvc";
        break;
    case INTEL:
        spec = "win32-icc";
        break;
    case MINGW:
        spec = "win32-g++";
        break;
    case BORLAND:
        spec = "win32-borland";
        break;
    default:
        break;
    }

    return spec;
}

/*!
    Returns the enum of the compiler which was detected on the system.
    The compilers are detected in the order as entered into the
    compiler_info list.

    If more than one compiler is found, Unknown is returned.
*/
Compiler Environment::detectCompiler()
{
#ifndef Q_OS_WIN32
    return MSVC6; // Always generate MSVC 6.0 versions on other platforms
#else
    if(detectedCompiler != Unknown)
        return detectedCompiler;

    int installed = 0;

    // Check for compilers in registry first, to see which version is in PATH
    QString paths = qgetenv("PATH");
    QStringList pathlist = paths.toLower().split(";");
    for(int i = 0; compiler_info[i].compiler; ++i) {
        QString productPath = readRegistryKey(HKEY_LOCAL_MACHINE, compiler_info[i].regKey).toLower();
        if (productPath.length()) {
            QStringList::iterator it;
            for(it = pathlist.begin(); it != pathlist.end(); ++it) {
                if((*it).contains(productPath)) {
                    ++installed;
                    detectedCompiler = compiler_info[i].compiler;
                    break;
                }
            }
        }
    }

    // Now just go looking for the executables, and accept any executable as the lowest version
    if (!installed) {
        for(int i = 0; compiler_info[i].compiler; ++i) {
            QString executable = QString(compiler_info[i].executable).toLower();
            if (executable.length() && Environment::detectExecutable(executable)) {
                ++installed;
                detectedCompiler = compiler_info[i].compiler;
                break;
            } 
        }
    }

    if (installed > 1) {
        cout << "Found more than one known compiler! Using \"" << compilerInfo(detectedCompiler)->compilerStr << "\"" << endl;
        detectedCompiler = Unknown;
    }
    return detectedCompiler;
#endif
};

/*!
    Returns true if the \a executable could be loaded, else false.
    This means that the executable either is in the current directory
    or in the PATH.
*/
bool Environment::detectExecutable(const QString &executable)
{
    PROCESS_INFORMATION procInfo;
    memset(&procInfo, 0, sizeof(procInfo));

    bool couldExecute;
    QT_WA({
        // Unicode version
        STARTUPINFOW startInfo;
        memset(&startInfo, 0, sizeof(startInfo));
        startInfo.cb = sizeof(startInfo);
        
        couldExecute = CreateProcessW(0, (WCHAR*)executable.utf16(),
                                      0, 0, false,
                                      CREATE_NO_WINDOW | CREATE_SUSPENDED,
                                      0, 0, &startInfo, &procInfo);
                        
    }, {
        // Ansi version 
        STARTUPINFOA startInfo;
        memset(&startInfo, 0, sizeof(startInfo));
        startInfo.cb = sizeof(startInfo);
        
        couldExecute = CreateProcessA(0, executable.toLocal8Bit().data(),
                                      0, 0, false, 
                                      CREATE_NO_WINDOW | CREATE_SUSPENDED,
                                      0, 0, &startInfo, &procInfo);
    })

    if (couldExecute) {
        CloseHandle(procInfo.hThread);
        TerminateProcess(procInfo.hProcess, 0);
        CloseHandle(procInfo.hProcess);
    }
    return couldExecute;
}

/*!
    Creates a commandling from \a program and it \a arguments,
    escaping characters that needs it.
*/
static QString qt_create_commandline(const QString &program, const QStringList &arguments)
{
    QString programName = program;
    if (!programName.startsWith("\"") && !programName.endsWith("\"") && programName.contains(" "))
        programName = "\"" + programName + "\"";
    programName.replace("/", "\\");

    QString args;
    // add the prgram as the first arrg ... it works better
    args = programName + " ";
    for (int i=0; i<arguments.size(); ++i) {
        QString tmp = arguments.at(i);
        // in the case of \" already being in the string the \ must also be escaped
        tmp.replace( "\\\"", "\\\\\"" );
        // escape a single " because the arguments will be parsed
        tmp.replace( "\"", "\\\"" );
        if (tmp.isEmpty() || tmp.contains(' ') || tmp.contains('\t')) {
            // The argument must not end with a \ since this would be interpreted
            // as escaping the quote -- rather put the \ behind the quote: e.g.
            // rather use "foo"\ than "foo\"
            QString endQuote("\"");
            int i = tmp.length();
            while (i>0 && tmp.at(i-1) == '\\') {
                --i;
                endQuote += "\\";
            }
            args += QString(" \"") + tmp.left(i) + endQuote;
        } else {
            args += ' ' + tmp;
        }
    }
    return args;
}

/*!
    Creates a QByteArray of the \a environment in either UNICODE or
    ansi representation.
*/
static QByteArray qt_create_environment(const QStringList &environment)
{
    QByteArray envlist;
    if (!environment.isEmpty()) {
	int pos = 0;
	// add PATH if necessary (for DLL loading)
	QByteArray path = qgetenv("PATH");
        QT_WA({
	    if (environment.filter(QRegExp("^PATH=",Qt::CaseInsensitive)).isEmpty()
                && !path.isNull()) {
                QString tmp = QString(QLatin1String("PATH=%1")).arg(QString::fromLocal8Bit(path));
                uint tmpSize = sizeof(TCHAR) * (tmp.length()+1);
                envlist.resize(envlist.size() + tmpSize );
                memcpy(envlist.data()+pos, tmp.utf16(), tmpSize);
                pos += tmpSize;
	    }
	    // add the user environment
	    for (QStringList::ConstIterator it = environment.begin(); it != environment.end(); it++ ) {
                QString tmp = *it;
                uint tmpSize = sizeof(TCHAR) * (tmp.length()+1);
                envlist.resize(envlist.size() + tmpSize);
                memcpy(envlist.data()+pos, tmp.utf16(), tmpSize);
                pos += tmpSize;
	    }
	    // add the 2 terminating 0 (actually 4, just to be on the safe side)
	    envlist.resize( envlist.size()+4 );
	    envlist[pos++] = 0;
	    envlist[pos++] = 0;
	    envlist[pos++] = 0;
	    envlist[pos++] = 0;
        }, {
            if (environment.filter(QRegExp("^PATH=",Qt::CaseInsensitive)).isEmpty() && !path.isNull()) {
                QByteArray tmp = QString("PATH=%1").arg(QString::fromLocal8Bit(path)).toLocal8Bit();
                uint tmpSize = tmp.length() + 1;
                envlist.resize(envlist.size() + tmpSize);
                memcpy(envlist.data()+pos, tmp.data(), tmpSize);
                pos += tmpSize;
            }
            // add the user environment
            for (QStringList::ConstIterator it = environment.begin(); it != environment.end(); it++) {
                QByteArray tmp = (*it).toLocal8Bit();
                uint tmpSize = tmp.length() + 1;
                envlist.resize(envlist.size() + tmpSize);
                memcpy(envlist.data()+pos, tmp.data(), tmpSize);
                pos += tmpSize;
            }
            // add the terminating 0 (actually 2, just to be on the safe side)
            envlist.resize(envlist.size()+2);
            envlist[pos++] = 0;
            envlist[pos++] = 0;
        })
    }

    return envlist;
}

/*!
    Executes the command described in \a arguments, in the
    environment inherited from the parent process, with the
    \a additionalEnv settings applied.
    \a removeEnv removes the specified environment variables from
    the environment of the executed process.

    Returns the exit value of the process, or -1 if the command could
    not be executed.

    This function uses _(w)spawnvpe to spawn a process by searching
    through the PATH environment variable.
*/
int Environment::execute(QStringList arguments, const QStringList &additionalEnv, const QStringList &removeEnv)
{
// GetEnvironmentStrings is defined to GetEnvironmentStringsW when
// UNICODE is defined. We cannot use that, since we need to
// destinguish between unicode and ansi versions of the functions.
#if defined(UNICODE) && defined(GetEnvironmentStrings)
#undef GetEnvironmentStrings
#endif

    // Create the full environment from the current environment and
    // the additionalEnv strings, then remove all variables defined
    // in removeEnv
    QMap<QString, QString> fullEnvMap;
    QT_WA({
        LPWSTR envStrings = GetEnvironmentStringsW();
        if (envStrings) {
            int strLen = 0;
            for (LPWSTR envString = envStrings; *(envString); envString += strLen + 1) {
                strLen = wcslen(envString);
                QString str = QString((const QChar*)envString, strLen);
                if (!str.startsWith("=")) { // These are added by the system
                    int sepIndex = str.indexOf('=');
                    fullEnvMap.insert(str.left(sepIndex).toUpper(), str.mid(sepIndex +1));
                }
            }
        }
        FreeEnvironmentStringsW(envStrings);
    }, {
        LPSTR envStrings = GetEnvironmentStrings();
        if (envStrings) {
            int strLen = 0;
            for (LPSTR envString = envStrings; *(envString); envString += strLen + 1) {
                strLen = strlen(envString);
                QString str = QLatin1String(envString);
                if (!str.startsWith("=")) { // These are added by the system
                    int sepIndex = str.indexOf('=');
                    fullEnvMap.insert(str.left(sepIndex).toUpper(), str.mid(sepIndex +1));
                }
            }
        }
        FreeEnvironmentStringsA(envStrings);
    })
    // Add additionalEnv variables
    for (int i = 0; i < additionalEnv.count(); ++i) {
        const QString &str = additionalEnv.at(i);
        int sepIndex = str.indexOf('=');
        fullEnvMap.insert(str.left(sepIndex).toUpper(), str.mid(sepIndex +1));
    }

    // Remove removeEnv variables
    for (int j = 0; j < removeEnv.count(); ++j)
        fullEnvMap.remove(removeEnv.at(j).toUpper());

    // Add all variables to a QStringList
    QStringList fullEnv;
    QMapIterator<QString, QString> it(fullEnvMap);
    while (it.hasNext()) {
        it.next();
        fullEnv += QString(it.key() + "=" + it.value());
    }

    // ----------------------------
    QString program = arguments.takeAt(0);
    QString args = qt_create_commandline(program, arguments);
    QByteArray envlist = qt_create_environment(fullEnv);

    DWORD exitCode = -1;
    PROCESS_INFORMATION procInfo;
    memset(&procInfo, 0, sizeof(procInfo));

    bool couldExecute;
    QT_WA({
        // Unicode version
        STARTUPINFOW startInfo;
        memset(&startInfo, 0, sizeof(startInfo));
        startInfo.cb = sizeof(startInfo);

        couldExecute = CreateProcessW(0, (WCHAR*)args.utf16(),
                                      0, 0, false, CREATE_UNICODE_ENVIRONMENT,
                                      envlist.isEmpty() ? 0 : envlist.data(),
                                      0, &startInfo, &procInfo);
    }, {
        // Ansi version 
        STARTUPINFOA startInfo;
        memset(&startInfo, 0, sizeof(startInfo));
        startInfo.cb = sizeof(startInfo);
        
        couldExecute = CreateProcessA(0, args.toLocal8Bit().data(),
                                      0, 0, false, 0,
                                      envlist.isEmpty() ? 0 : envlist.data(),
                                      0, &startInfo, &procInfo);
    })

    if (couldExecute) {
        WaitForSingleObject(procInfo.hProcess, INFINITE);
        GetExitCodeProcess(procInfo.hProcess, &exitCode);
        CloseHandle(procInfo.hThread);
        CloseHandle(procInfo.hProcess);
    }


    if (exitCode == -1) {
        switch(GetLastError()) {
        case E2BIG:
            cerr << "execute: Argument list exceeds 1024 bytes" << endl;
            foreach(QString arg, arguments)
                cerr << "   (" << arg.toLocal8Bit().constData() << ")" << endl;
            break;
        case ENOENT:
            cerr << "execute: File or path is not found (" << program.toLocal8Bit().constData() << ")" << endl;
            break;
        case ENOEXEC:
            cerr << "execute: Specified file is not executable or has invalid executable-file format (" << program.toLocal8Bit().constData() << ")" << endl;
            break;
        case ENOMEM:
            cerr << "execute: Not enough memory is available to execute new process." << endl;
            break;
        default:
            cerr << "execute: Unknown error" << endl;
            foreach(QString arg, arguments)
                cerr << "   (" << arg.toLocal8Bit().constData() << ")" << endl;
            break;
        }
    }
    return exitCode;
}

