/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef X11KEYFAKER_H
#define X11KEYFAKER_H

#include <QObject>
#include <QX11Info>
#include <qnamespace.h>

QT_BEGIN_NAMESPACE

class X11KeyFaker : public QObject
{
    Q_OBJECT
public:
    X11KeyFaker(const QString& displayName, QObject *parent = 0);
    ~X11KeyFaker();

    bool isConnected() const { return dpy != 0; }

    void sendKeyEvent(int qtCode, bool isPress);

private slots:
    void connect();
    void readyRead();

signals:
    void connected();
    void couldNotConnect();

private:
    QString displayName;
    Display *dpy;
    int retryCount;
    int shiftKeycode;
    int modeSwitchKeycode;
    int modifiers;
};

QT_END_NAMESPACE

#endif
