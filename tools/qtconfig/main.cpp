/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the $MODULE$ of the Qt Toolkit.
**
** $LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QApplication>
#include <QString>
#include <QByteArray>
#include <QLibraryInfo>
#include <QTranslator>
#include <QLatin1String>
#include <QLocale>
#include <QStringList>
#include <QDebug>
#include "window.h"

int main(int argc, char **argv)
{
    QApplication a(argc, argv);
    QStringList translationDir;
    for (int i=1; i<argc; ++i) {
        const QByteArray tmp(argv[i]);
        if (tmp == "-resourcedir") {
            if (++i >= argc) {
                qWarning("-resource requires an argument");
                return 1;
            }
            translationDir.append(QString::fromAscii(argv[i]));
        } else if (tmp == "-help" || tmp == "-h" || tmp == "--help") {
            printf("Usage: qtconfig [options]\n"
                   " -h|-help|--help             Show this help\n"
                   " -resourcedir                Qt config will load translations from this directory\n");
            return 0;
        } else {
            qWarning("Unknown option '%s'", argv[i]);
        }
    }
    translationDir.append(QLibraryInfo::location(QLibraryInfo::TranslationsPath));
    translationDir.append(QLatin1String("."));
    QString localeName = QLocale::system().name();
    if (localeName.size() > 2)
        localeName = localeName.left(2);
    QTranslator qt, qtconfig;
    for (int i=0; i<translationDir.size(); ++i) {
        if (qtconfig.load(QLatin1String("qtconfig_") + localeName, translationDir.at(i))) {
            a.installTranslator(&qtconfig);
            break;
        }
    }
    for (int i=0; i<translationDir.size(); ++i) {
        if (qt.load(QLatin1String("qt_") + localeName, translationDir.at(i))) {
            a.installTranslator(&qt);
            break;
        }
    }

    Window w;
    w.show();
    return a.exec();
}
