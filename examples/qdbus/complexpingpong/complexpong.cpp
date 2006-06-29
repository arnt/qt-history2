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

#include <stdio.h>
#include <stdlib.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtDBus/QtDBus>

#include "ping-common.h"
#include "complexpong.h"

// the property
QString Pong::value() const
{
    return m_value;
}

void Pong::setValue(const QString &newValue)
{
    m_value = newValue;
}

void Pong::quit()
{
    QTimer::singleShot(0, QCoreApplication::instance(), SLOT(quit()));
}

QDBusVariant Pong::query(const QString &query)
{
    QString q = query.toLower();
    if (q == "hello")
        return QDBusVariant("World");
    if (q == "ping")
        return QDBusVariant("Pong");
    if (q == "the answer to life, the universe and everything")
        return QDBusVariant(42);
    if (q.indexOf("unladen swallow") != -1) {
        if (q.indexOf("european") != -1)
            return QDBusVariant(11.0);
        return QDBusVariant(QByteArray("african or european?"));
    }

    return QDBusVariant("Sorry, I don't know the answer");
}

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    QObject obj;
    Pong *pong = new Pong(&obj);
    pong->connect(&app, SIGNAL(aboutToQuit()), SIGNAL(aboutToQuit()));
    pong->setProperty("value", "initial value");
    QDBus::sessionBus().registerObject("/", &obj);

    if (!QDBus::sessionBus().registerService(SERVICE_NAME)) {
        fprintf(stderr, "%s\n",
                qPrintable(QDBus::sessionBus().lastError().message()));        
        exit(1);
    }
    
    app.exec();
    return 0;
}

