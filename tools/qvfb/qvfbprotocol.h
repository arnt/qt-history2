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
#ifndef QVFBPROTOCOL_H
#define QVFBPROTOCOL_H

#include <QImage>
#include <QVector>
#include <QColor>
class QVFbKeyProtocol;
class QVFbPointerProtocol;
class QVFbViewProtocol : public QObject
{
    Q_OBJECT
public:
    QVFbViewProtocol(int display_id, QObject *parent = 0)
        : QObject(parent), mDisplayId(display_id) {}

    virtual ~QVFbViewProtocol() {}

    int id() const { return mDisplayId; }
    virtual QSize size() const = 0;
    virtual int depth() const = 0;

    virtual QImage image(const QRect & = QRect()) const = 0;

    /* for paletted depths */
    virtual QVector<QColor> colorTable() const = 0;
    virtual void setColorTable(QVector<QColor> &table) const = 0;

    virtual QVFbKeyProtocol *keyHandler() const = 0;
    virtual QVFbPointerProtocol *mouseHandler() const = 0;

signals:
    void displayDataChanged(const QRect &);
    void displayModeChanged(const QSize &size, int depth);

private:
    int mDisplayId;
};

class QVFbKeyProtocol
{
public:
    QVFbKeyProtocol(int display_id) : mDisplayId(display_id) {}
    virtual ~QVFbKeyProtocol() {}

    int id() const { return mDisplayId; }

    virtual void sendKeyboardData(int unicode, int keycode,
            int modifiers, bool press, bool repeat) = 0;

private:
    int mDisplayId;
};

class QVFbPointerProtocol
{
public:
    QVFbPointerProtocol(int display_id) : mDisplayId(display_id) {}
    virtual ~QVFbPointerProtocol() {}

    int id() const { return mDisplayId; }

    virtual void sendPointerData(QPoint &pos, int buttons, int wheel) = 0;

private:
    int mDisplayId;
};

/* since there is very little variation in input protocols defaults are
   provided */

class QVFbKeyPipeProtocol : public QVFbKeyProtocol
{
public:
    QVFbKeyPipeProtocol(int display_id);
    ~QVFbKeyPipeProtocol();

    void sendKeyboardData(int unicode, int keycode,
            int modifiers, bool press, bool repeat);

private:
    int fd;
    QString fileName;
};

class QVFbPointerPipeProtocol : public QVFbPointerProtocol
{
public:
    QVFbPointerPipeProtocol(int display_id, bool sendWheelEvents);
    ~QVFbPointerPipeProtocol();

    void sendPointerData(QPoint &pos, int buttons, int wheel);

    bool wheelEventsSupported() const { return mSupportWheelEvents; }

private:
    int fd;
    QString fileName;
    bool mSupportWheelEvents;
};

#endif
