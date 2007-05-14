/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ $TROLLTECH$. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/


#include <qscriptengine.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QStringList>
#include <QtGui/QApplication>

#include <stdlib.h>

static void interactive(QScriptEngine &eng)
{
    QTextStream qin(stdin, QFile::ReadOnly);

    const char *qscript_prompt = "qs> ";
    const char *dot_prompt = ".... ";
    const char *prompt = qscript_prompt;

    QString code;

    forever {
        QString line;

        printf("%s", prompt);
        fflush(stdout);

        line = qin.readLine();
        if (line.isNull())
            break;

        code += line;
        code += QLatin1Char('\n');

        if (line.trimmed().isEmpty()) {
            continue;

        } else if (! eng.canEvaluate(code)) {
            prompt = dot_prompt;

        } else {
            QScriptValue result = eng.evaluate(code, QLatin1String("typein"));

            code.clear();
            prompt = qscript_prompt;

            if (! result.isUndefined())
                fprintf(stderr, "%s\n", qPrintable(result.toString()));
        }
    }
}

static QScriptValue importExtension(QScriptContext *context, QScriptEngine *engine)
{
    return engine->importExtension(context->argument(0).toString());
}

int main(int argc, char *argv[])
{
    QApplication *app;
    if (argc >= 2 && !qstrcmp(argv[1], "-tty"))
        app = new QApplication(argc, argv, QApplication::Tty);
    else
        app = new QApplication(argc, argv);

    QScriptEngine eng;

    eng.importExtension("qt.core");
    eng.importExtension("qt.gui");

    QScriptValue globalObject = eng.globalObject();

    {
        QScriptValue qscript = eng.newObject();
        qscript.setProperty("importExtension", eng.newFunction(importExtension));
        globalObject.property("qt").setProperty("script", qscript);
    }

    if (! *++argv) {
        interactive(eng);
        return EXIT_SUCCESS;
    }

    while (const char *arg = *argv++) {
        QString fn = QString::fromLocal8Bit(arg);

        if (fn == QLatin1String("-i")) {
            interactive(eng);
            break;
        }

        QString contents;
        int lineNumber = 1;

        if (fn == QLatin1String("-")) {
            QTextStream stream(stdin, QFile::ReadOnly);
            contents = stream.readAll();
        }

        else {
            QFile file(fn);

            if (file.open(QFile::ReadOnly)) {
                QTextStream stream(&file);
                contents = stream.readAll();
                file.close();

                // strip off #!/usr/bin/env qscript line
                if (contents.startsWith("#!")) {
                    contents.remove(0, contents.indexOf("\n"));
                    ++lineNumber;
                }
            }
        }

        if (contents.isEmpty())
            continue;

        QScriptValue r = eng.evaluate(contents, fn, lineNumber);
        if (eng.hasUncaughtException()) {
            QStringList backtrace = qscriptvalue_cast<QStringList>(r.property("backtrace").call(r));
            fprintf (stderr, "    %s\n%s\n\n", qPrintable(r.toString()),
                     qPrintable(backtrace.join("\n")));
            return EXIT_FAILURE;
        }
    }

    delete app;

    return EXIT_SUCCESS;
}
