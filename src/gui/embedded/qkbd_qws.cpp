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

#include "qkbd_qws.h"

#ifndef QT_NO_QWS_KEYBOARD

#include "qwindowsystem_qws.h"
#include "qscreen_qws.h"
#include "qtimer.h"
#include <stdlib.h>


class QWSKbPrivate : public QObject
{
    Q_OBJECT
public:
    QWSKbPrivate(QWSKeyboardHandler *h) {
        handler = h;
        arTimer = new QTimer(this);
        arTimer->setSingleShot(true);
        connect(arTimer, SIGNAL(timeout()), SLOT(autoRepeat()));
        repeatdelay = 400;
        repeatperiod = 80;
    }

    void beginAutoRepeat(int uni, int code, Qt::KeyboardModifiers mod) {
        unicode = uni;
        keycode = code;
        modifier = mod;
        arTimer->start(repeatdelay);
    }
    void endAutoRepeat() {
        arTimer->stop();
    }

private slots:
    void autoRepeat() {
        handler->processKeyEvent(unicode, keycode, modifier, false, true);
        handler->processKeyEvent(unicode, keycode, modifier, true, true);
        arTimer->start(repeatperiod);
    }

private:
    QWSKeyboardHandler *handler;
    int unicode;
    int keycode;
    Qt::KeyboardModifiers modifier;
    int repeatdelay;
    int repeatperiod;
    QTimer *arTimer;
};

/*!
    \class QWSKeyboardHandler qkbd_qws.h
    \brief The QWSKeyboardHandler class implements the keyboard driver
    for Qtopia Core.

    \ingroup qws

    The keyboard driver handles events from system devices and
    generates key events.

    A QWSKeyboardHandler will usually open some system device in its
    constructor, create a QSocketNotifier on that opened device and
    when it receives data, it will call processKeyEvent() to send the
    event to Qtopia Core for relaying to clients.
*/


/*!
    Constructs a keyboard handler. The handler \e may be passed to the
    system for later destruction with QWSServer::setKeyboardHandler(),
    although even without doing this, the handler can function,
    calling processKeyEvent() to emit events.
*/
QWSKeyboardHandler::QWSKeyboardHandler()
{
    d = new QWSKbPrivate(this);
}

/*!
    Destroys a keyboard handler. Note that if you have called
    QWSServer::setKeyboardHandler(), you must not delete the handler.
*/
QWSKeyboardHandler::~QWSKeyboardHandler()
{
    delete d;
}


/*!
    Subclasses call this function to send a key event. The server may
    additionally filter the event before sending it on to
    applications.

    \table
    \header \i Parameter \i Meaning
    \row \i \a unicode
         \i The Unicode value for the key, or 0xFFFF is none is appropriate.
    \row \i \a keycode
         \i The Qt keycode for the key (see \l{Qt::Key} for the list of codes).
    \row \i \a modifiers
         \i The set of modifier keys (see \l{Qt::Modifier}).
    \row \i \a isPress
         \i Whether this is a press or a release.
    \row \i \a autoRepeat
         \i Whether this event was generated by an auto-repeat
            mechanism, or an actual key press.
    \endtable
*/
void QWSKeyboardHandler::processKeyEvent(int unicode, int keycode, Qt::KeyboardModifiers modifiers,
                        bool isPress, bool autoRepeat)
{
    qwsServer->processKeyEvent(unicode, keycode, modifiers, isPress, autoRepeat);
}

/*!
    Transforms an arrow key with keycode \a key (\c Qt::Key_Left, \c Qt::Key_Up, \c Qt::Key_Right, \c
    Qt::Key_Down) to the orientation of the display and returns the transformed keycode.
 */
int QWSKeyboardHandler::transformDirKey(int key)
{
    static int dir_keyrot = -1;
    if (dir_keyrot < 0) {
        // get the rotation
        switch (qgetenv("QWS_CURSOR_ROTATION").toInt()) {
        case 90: dir_keyrot = 1; break;
        case 180: dir_keyrot = 2; break;
        case 270: dir_keyrot = 3; break;
        default: dir_keyrot = 0; break;
        }
    }
    int xf = qt_screen->transformOrientation() + dir_keyrot;
    return (key-Qt::Key_Left+xf)%4+Qt::Key_Left;
}

/*!
    Begin auto repeating the specified key press. After a short delay
    the key sequence will be sent periodically until endAutoRepeat()
    is called.

    \a uni is the unicode value, \a code is the keycode and \a mod is the modifier state of the key sequence.

    \sa endAutoRepeat()
*/
void QWSKeyboardHandler::beginAutoRepeat(int uni, int code, Qt::KeyboardModifiers mod)
{
    d->beginAutoRepeat(uni, code, mod);
}

/*!
    Stop auto-repeating a key press.

    \sa beginAutoRepeat()
*/
void QWSKeyboardHandler::endAutoRepeat()
{
    d->endAutoRepeat();
}

#include "qkbd_qws.moc"

#endif // QT_NO_QWS_KEYBOARD

