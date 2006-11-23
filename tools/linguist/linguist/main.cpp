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

#include "trwindow.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QPixmap>
#include <QTextCodec>
#include <QTranslator>
#include <QSettings>
#include <QSplashScreen>
#include <QLibraryInfo>
#include <QLocale>
#include <QFile>

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(linguist);

    QApplication app(argc, argv);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QStringList files;
    QString resourceDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    QStringList args = app.arguments();

    for (int i = 1; i < args.count(); ++i)
    {
        QString argument = args.at(i);
        if (argument == QLatin1String("-resourcedir")) {
            if (i + 1 < args.count()) {
                resourceDir = QFile::decodeName(args.at(++i).toLocal8Bit());
            } else {
                // issue a warning
            }
        } else if (!files.contains(argument)) {
            files.append(argument);
        }
    }

    QTranslator translator(0);
    translator.load(QLatin1String("linguist_") + QLocale::system().name(), resourceDir);
    app.installTranslator(&translator);

    QTranslator qtTranslator(0);
    qtTranslator.load(QLatin1String("qt_") + QLocale::system().name(), resourceDir);
    app.installTranslator(&qtTranslator);

    app.setOrganizationName(QLatin1String("Trolltech"));
    app.setApplicationName(QLatin1String("Linguist"));
    QString keybase(QString::number( (QT_VERSION >> 16) & 0xff ) +
                     QLatin1Char('.') + QString::number( (QT_VERSION >> 8) & 0xff ) + QLatin1Char('/') );
    QSettings config;

    QWidget tmp;
    tmp.restoreGeometry(config.value(keybase + QLatin1String("Geometry/WindowGeometry")).toByteArray());
    
    QSplashScreen *splash = 0;
    int screenId = QApplication::desktop()->screenNumber(tmp.geometry().center());
    splash = new QSplashScreen(QApplication::desktop()->screen(screenId),
        QPixmap(QLatin1String(":/images/splash.png")));
    if (QApplication::desktop()->isVirtualDesktop()) {
        QRect srect(0, 0, splash->width(), splash->height());
        splash->move(QApplication::desktop()->availableGeometry(screenId).center() - srect.center() );
    }
    splash->setAttribute(Qt::WA_DeleteOnClose);
    splash->show();

    TrWindow tw;
    tw.show();

    splash->finish(&tw);

    if (files.count())
        tw.openFile(files.last());

    QApplication::restoreOverrideCursor();

    return app.exec();
}
