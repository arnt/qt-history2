/****************************************************************************
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
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
#include <QLocale>

int main(int argc, char **argv)
{
    Q_INIT_RESOURCE(linguist);

    QApplication app(argc, argv);
    QApplication::setOverrideCursor(Qt::WaitCursor);

    QString lang = QLocale::system().name();
    lang.chop(3); //remove country
    QTranslator translator(0);
    translator.load(QString("linguist_") + lang, ".");

    app.installTranslator(&translator);

    app.setOrganizationName("Trolltech");
    app.setApplicationName("Linguist");
    QString keybase(QString::number( (QT_VERSION >> 16) & 0xff ) +
                     "." + QString::number( (QT_VERSION >> 8) & 0xff ) + "/" );
    QSettings config;

    QRect r(QApplication::desktop()->availableGeometry());
    r.setX(config.value(keybase + "Geometry/MainwindowX", r.x()).toInt());
    r.setY(config.value(keybase + "Geometry/MainwindowY", r.y()).toInt());
    r.setWidth(config.value(keybase + "Geometry/MainwindowWidth", r.width()).toInt());
    r.setHeight(config.value(keybase + "Geometry/MainwindowHeight", r.height()).toInt());
    if (!r.intersects(QApplication::desktop()->geometry()))
        r.moveTopLeft(QApplication::desktop()->availableGeometry().topLeft());

    QSplashScreen *splash = 0;
    int screenId = QApplication::desktop()->screenNumber(r.center());
    splash = new QSplashScreen(QApplication::desktop()->screen(screenId),
        QPixmap(":/images/splash.png"));
    if (QApplication::desktop()->isVirtualDesktop()) {
        QRect srect(0, 0, splash->width(), splash->height());
        splash->move(QApplication::desktop()->availableGeometry(screenId).center() - srect.center() );
    }
    splash->setAttribute(Qt::WA_DeleteOnClose);
    splash->show();

    TrWindow tw;

    if (config.value(keybase + "Geometry/MainwindowMaximized", false).toBool())
        tw.setWindowState(tw.windowState() | Qt::WindowMaximized);
    tw.show();

    if (splash)
        splash->finish(&tw);

    if (app.argc() > 1)
        tw.openFile(QString(app.argv()[app.argc() - 1]));

    QApplication::restoreOverrideCursor();

    return app.exec();
}
