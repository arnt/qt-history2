
#include <qmakeinterface.h>

int
main(int argc, char **argv)
{
#if 0
    QMakeInterface qmake(argc == 2 ? QString(argv[1]) : QString());
#else
    QMakeInterface qmake(false, argc, argv);
#endif
    qDebug("CXXFLAGS = %s", qmake.compileFlags().join("::").latin1());
    qDebug("CFLAGS = %s", qmake.compileFlags(false).join("::").latin1());
    qDebug("DEFINES = %s", qmake.defines().join("::").latin1());
    qDebug("LFLAGS = %s", qmake.linkFlags().join("::").latin1());
    qDebug("LIBS = %s", qmake.libraries().join("::").latin1());
    qDebug("builds: %s", qmake.buildStyles().join(", ").latin1());
    QString deb = qmake.buildStyles().first();
    qDebug("LIBS = %s", qmake.libraries(deb).join("::").latin1());
    return 1;
}
