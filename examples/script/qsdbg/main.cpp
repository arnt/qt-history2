#include <QtScript>

#include "scriptdebugger.h"

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "*** you must specify a script file to evaluate (try example.qs)\n");
        return(-1);
    }

    QString fileName = QString::fromLatin1(argv[1]);
    QFile file(fileName);
    if (!file.open(QFile::ReadOnly)) {
        fprintf(stderr, "*** failed to open `%s' for reading\n", argv[1]);
        return(-1);
    }

    QScriptEngine engine;
    QString code = QTextStream(&file).readAll();
    file.close();

    fprintf(stdout, "\n*** Welcome to qsdbg. Debugger commands start with a . (period)\n");
    fprintf(stdout, "*** Any other input will be evaluated by the script interpreter.\n");
    fprintf(stdout, "*** Type .help for help.\n\n");

    ScriptDebugger *dbg = new ScriptDebugger(&engine);
    dbg->breakAtNextStatement();

    engine.evaluate(code, fileName);

    return 0;
}
