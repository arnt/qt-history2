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

#ifndef QTESTEVENT_H
#define QTESTEVENT_H

#if 0
// inform syncqt
#pragma qt_no_master_include
#endif

#include <QtTest/qtest_global.h>
#include <QtTest/qtestkeyboard.h>
#include <QtTest/qtestmouse.h>
#include <QtTest/qtestsystem.h>

#include <QtCore/qlist.h>

#include <stdlib.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QTestEvent
{
public:
    virtual void simulate(QWidget *w) = 0;
    virtual QTestEvent *clone() const = 0;

    virtual ~QTestEvent() {}
};

class QTestKeyEvent: public QTestEvent
{
public:
    inline QTestKeyEvent(QTest::KeyAction action, Qt::Key key, Qt::KeyboardModifiers modifiers, int delay)
        : _action(action), _delay(delay), _modifiers(modifiers), _ascii(0), _key(key) {}
    inline QTestKeyEvent(QTest::KeyAction action, char ascii, Qt::KeyboardModifiers modifiers, int delay)
        : _action(action), _delay(delay), _modifiers(modifiers),
          _ascii(ascii), _key(Qt::Key_unknown) {}
    inline QTestEvent *clone() const { return new QTestKeyEvent(*this); }

    inline void simulate(QWidget *w)
    {
        if (_ascii == 0)
            QTest::keyEvent(_action, w, _key, _modifiers, _delay);
        else
            QTest::keyEvent(_action, w, _ascii, _modifiers, _delay);
    }

protected:
    QTest::KeyAction _action;
    int _delay;
    Qt::KeyboardModifiers _modifiers;
    char _ascii;
    Qt::Key _key;
};

class QTestKeyClicksEvent: public QTestEvent
{
public:
    inline QTestKeyClicksEvent(const QString &keys, Qt::KeyboardModifiers modifiers, int delay)
        : _keys(keys), _modifiers(modifiers), _delay(delay) {}
    inline QTestEvent *clone() const { return new QTestKeyClicksEvent(*this); }

    inline void simulate(QWidget *w)
    {
        QTest::keyClicks(w, _keys, _modifiers, _delay);
    }

private:
    QString _keys;
    Qt::KeyboardModifiers _modifiers;
    int _delay;
};

class QTestMouseEvent: public QTestEvent
{
public:
    inline QTestMouseEvent(QTest::MouseAction action, Qt::MouseButton button,
            Qt::KeyboardModifiers modifiers, QPoint position, int delay)
        : _action(action), _button(button), _modifiers(modifiers), _pos(position), _delay(delay) {}
    inline QTestEvent *clone() const { return new QTestMouseEvent(*this); }

    inline void simulate(QWidget *w)
    {
        QTest::mouseEvent(_action, w, _button, _modifiers, _pos, _delay);
    }

private:
    QTest::MouseAction _action;
    Qt::MouseButton _button;
    Qt::KeyboardModifiers _modifiers;
    QPoint _pos;
    int _delay;
};

class QTestDelayEvent: public QTestEvent
{
public:
    inline QTestDelayEvent(int msecs): _delay(msecs) {}
    inline QTestEvent *clone() const { return new QTestDelayEvent(*this); }

    inline void simulate(QWidget * /*w*/) { QTest::qWait(_delay); }

private:
    int _delay;
};

class QTestEventList: public QList<QTestEvent *>
{
public:
    inline QTestEventList() {}
    inline QTestEventList(const QTestEventList &other): QList<QTestEvent *>()
    { for (int i = 0; i < other.count(); ++i) append(other.at(i)->clone()); }
    inline ~QTestEventList()
    { clear(); }
    inline void clear()
    { qDeleteAll(*this); QList<QTestEvent *>::clear(); }

    inline void addKeyClick(Qt::Key qtKey, Qt::KeyboardModifiers modifiers = Qt::NoModifier, int msecs = -1)
    { addKeyEvent(QTest::Click, qtKey, modifiers, msecs); }
    inline void addKeyPress(Qt::Key qtKey, Qt::KeyboardModifiers modifiers = Qt::NoModifier, int msecs = -1)
    { addKeyEvent(QTest::Press, qtKey, modifiers, msecs); }
    inline void addKeyRelease(Qt::Key qtKey, Qt::KeyboardModifiers modifiers = Qt::NoModifier, int msecs = -1)
    { addKeyEvent(QTest::Release, qtKey, modifiers, msecs); }
    inline void addKeyEvent(QTest::KeyAction action, Qt::Key qtKey,
                            Qt::KeyboardModifiers modifiers = Qt::NoModifier, int msecs = -1)
    { append(new QTestKeyEvent(action, qtKey, modifiers, msecs)); }

    inline void addKeyClick(char ascii, Qt::KeyboardModifiers modifiers = Qt::NoModifier, int msecs = -1)
    { addKeyEvent(QTest::Click, ascii, modifiers, msecs); }
    inline void addKeyPress(char ascii, Qt::KeyboardModifiers modifiers = Qt::NoModifier, int msecs = -1)
    { addKeyEvent(QTest::Press, ascii, modifiers, msecs); }
    inline void addKeyRelease(char ascii, Qt::KeyboardModifiers modifiers = Qt::NoModifier, int msecs = -1)
    { addKeyEvent(QTest::Release, ascii, modifiers, msecs); }
    inline void addKeyClicks(const QString &keys, Qt::KeyboardModifiers modifiers = Qt::NoModifier, int msecs = -1)
    { append(new QTestKeyClicksEvent(keys, modifiers, msecs)); }
    inline void addKeyEvent(QTest::KeyAction action, char ascii, Qt::KeyboardModifiers modifiers = Qt::NoModifier, int msecs = -1)
    { append(new QTestKeyEvent(action, ascii, modifiers, msecs)); }

    inline void addMousePress(Qt::MouseButton button, Qt::KeyboardModifiers stateKey = 0,
                              QPoint pos = QPoint(), int delay=-1)
    { append(new QTestMouseEvent(QTest::MousePress, button, stateKey, pos, delay)); }
    inline void addMouseRelease(Qt::MouseButton button, Qt::KeyboardModifiers stateKey = 0,
                                QPoint pos = QPoint(), int delay=-1)
    { append(new QTestMouseEvent(QTest::MouseRelease, button, stateKey, pos, delay)); }
    inline void addMouseClick(Qt::MouseButton button, Qt::KeyboardModifiers stateKey = 0,
                              QPoint pos = QPoint(), int delay=-1)
    { append(new QTestMouseEvent(QTest::MouseClick, button, stateKey, pos, delay)); }
    inline void addMouseDClick(Qt::MouseButton button, Qt::KeyboardModifiers stateKey = 0,
                            QPoint pos = QPoint(), int delay=-1)
    { append(new QTestMouseEvent(QTest::MousePress, button, stateKey, pos, delay)); }
    inline void addMouseMove(QPoint pos = QPoint(), int delay=-1)
    { append(new QTestMouseEvent(QTest::MouseMove, Qt::NoButton, 0, pos, delay)); }

    inline void addDelay(int msecs)
    { append(new QTestDelayEvent(msecs)); }

    inline void simulate(QWidget *w)
    {
        for (int i = 0; i < count(); ++i)
            at(i)->simulate(w);
    }
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QTestEventList)

QT_END_HEADER

#endif
