#ifndef CONSOLE_H
#define CONSOLE_H
#include <QtCore/QTextStream>
#include <QtCore/QString>
#include <stdio.h>

struct Console {
    static void out(const QString &message) {
        QTextStream stream(stdout);
        stream << message;
    }
};

#endif // CONSOLE_H