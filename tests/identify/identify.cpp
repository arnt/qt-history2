#include <qapplication.h>
#include <stdio.h>

int main( int argc, char **argv )
{
    QApplication a(argc,argv);
#if defined(Q_OS_AIX)
    printf("Operating system: Q_OS_AIX\n");
#endif
#if defined(Q_OS_BSDI)
    printf("Operating system: Q_OS_BSDI\n");
#endif
#if defined(Q_OS_DGUX)
    printf("Operating system: Q_OS_DGUX\n");
#endif
#if defined(Q_OS_FREEBSD)
    printf("Operating system: Q_OS_FREEBSD\n");
#endif
#if defined(Q_OS_GNU)
    printf("Operating system: Q_OS_GNU\n");
#endif
#if defined(Q_OS_HPUX)
    printf("Operating system: Q_OS_HPUX\n");
#endif
#if defined(Q_OS_IRIX)
    printf("Operating system: Q_OS_IRIX\n");
#endif
#if defined(Q_OS_LINUX)
    printf("Operating system: Q_OS_LINUX\n");
#endif
#if defined(Q_OS_LYNXOS)
    printf("Operating system: Q_OS_LYNXOS\n");
#endif
#if defined(Q_OS_MAC)
    printf("Operating system: Q_OS_MAC\n");
#endif
#if defined(Q_OS_MSDOS)
    printf("Operating system: Q_OS_MSDOS\n");
#endif
#if defined(Q_OS_NETBSD)
    printf("Operating system: Q_OS_NETBSD\n");
#endif
#if defined(Q_OS_OPENBSD)
    printf("Operating system: Q_OS_OPENBSD\n");
#endif
#if defined(Q_OS_OS2EMX)
    printf("Operating system: Q_OS_OS2EMX\n");
#endif
#if defined(Q_OS_OS2)
    printf("Operating system: Q_OS_OS2\n");
#endif
#if defined(Q_OS_OSF)
    printf("Operating system: Q_OS_OSF\n");
#endif
#if defined(Q_OS_QNX6)
    printf("Operating system: Q_OS_QNX6\n");
#endif
#if defined(Q_OS_QNX4)
    printf("Operating system: Q_OS_QNX4\n");
#endif
#if defined(Q_OS_SCO)
    printf("Operating system: Q_OS_SCO\n");
#endif
#if defined(Q_OS_SOLARIS)
    printf("Operating system: Q_OS_SOLARIS\n");
#endif
#if defined(Q_OS_SUN)
    printf("Operating system: Q_OS_SUN\n");
#endif
#if defined(Q_OS_ULTRIX)
    printf("Operating system: Q_OS_ULTRIX\n");
#endif
#if defined(Q_OS_UNIXWARE7)
    printf("Operating system: Q_OS_UNIXWARE7\n");
#endif
#if defined(Q_OS_UNIXWARE)
    printf("Operating system: Q_OS_UNIXWARE\n");
#endif
#if defined(Q_OS_WIN32)
    printf("Operating system: Q_OS_WIN32\n");
#endif
#if defined(_OS_x_)
    printf("Operating system: _OS_x_\n");
#endif
#if defined(Q_CC_BOOLDEF_)
    printf("Compiler: Q_CC_BOOLDEF_\n");
#endif
#if defined(Q_CC_BOR)
    printf("Compiler: Q_CC_BOR\n");
#endif
#if defined(Q_CC_COMEAU)
    printf("Compiler: Q_CC_COMEAU\n");
#endif
#if defined(Q_CC_DEC)
    printf("Compiler: Q_CC_DEC\n");
#endif
#if defined(Q_CC_EDG)
    printf("Compiler: Q_CC_EDG\n");
#endif
#if defined(Q_CC_GNU)
    printf("Compiler: Q_CC_GNU\n");
#endif
#if defined(Q_CC_HP)
    printf("Compiler: Q_CC_HP\n");
#endif
#if defined(Q_CC_HPACC_)
    printf("Compiler: Q_CC_HPACC_\n");
#endif
#if defined(Q_CC_METROWORKS)
    printf("Compiler: Q_CC_METROWORKS\n");
#endif
#if defined(Q_CC_MPW)
    printf("Compiler: Q_CC_MPW\n");
#endif
#if defined(Q_CC_MSVC)
    printf("Compiler: Q_CC_MSVC\n");
#endif
#if defined(Q_CC_OC)
    printf("Compiler: Q_CC_OC\n");
#endif
#if defined(Q_CC_SUN)
    printf("Compiler: Q_CC_SUN\n");
#endif
#if defined(Q_CC_SYM)
    printf("Compiler: Q_CC_SYM\n");
#endif
#if defined(Q_CC_USLC)
    printf("Compiler: Q_CC_USLC\n");
#endif
#if defined(Q_CC_WAT)
    printf("Compiler: Q_CC_WAT\n");
#endif
#if defined(Q_CC_XLC)
    printf("Compiler: Q_CC_XLC\n");
#endif
#if defined(_CC_x_)
    printf("Compiler: _CC_x_\n");
#endif
#if defined(Q_WS_WIN)
    printf("Windows version: ");
    switch ( QApplication::winVersion() ) {
        case WV_NT:
            printf("Windows NT");
            break;
        case WV_95:
            printf("Windows 95");
            break;
        case WV_98:
            printf("Windows 98");
            break;
        case WV_32s:
            printf("Windows 32s");
            break;
        default:
            printf("Unknown");
            break;
    }
#endif
    return 0;
}
