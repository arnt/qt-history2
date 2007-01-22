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

#if defined(WITH_DBUS)
#include "qdbusbinding.h"
#endif

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
            QScriptValue result = eng.evaluate(code);

            code.clear();
            prompt = qscript_prompt;

            if (! result.isUndefined())
                fprintf(stderr, "%s\n", qPrintable(result.toString()));
        }
    }
}

int main(int, char *argv[])
{
    QScriptEngine eng;
    QScriptValue globalObject = eng.globalObject();

#if defined(WITH_DBUS)
    registerDBusBindings(&eng);
#endif

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
            }
        }

        if (contents.isEmpty())
            continue;

        QScriptValue r = eng.evaluate(contents);
        if (eng.hasUncaughtException()) {
            int line = eng.uncaughtExceptionLineNumber();
            fprintf (stderr, "%d: %s\n\t%s\n\n", line, qPrintable(fn), qPrintable(r.toString()));
            return EXIT_FAILURE;
        }
    }

    return EXIT_SUCCESS;
}
