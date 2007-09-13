#ifndef CONSOLE_H
#define CONSOLE_H
#include <QtCore/QTextStream>
#include <QtCore/QString>
#include <stdio.h>

QT_BEGIN_NAMESPACE

struct Console {
    static void out(const QString &message) {
        QTextStream stream(stdout);
        stream << message;
    }
};

QT_END_NAMESPACE

#endif // CONSOLE_H
