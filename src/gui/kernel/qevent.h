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

#ifndef QEVENT_H
#define QEVENT_H

#include "qwindowdefs.h"
#include "qobject.h"
#include "qregion.h"
#include "qnamespace.h"
#include "qmime.h"
#include "qpair.h"
#include "qstring.h"
#include "qkeysequence.h"
#include "qcoreevent.h"

class QAction;

class Q_GUI_EXPORT QInputEvent : public QEvent
{
public:
    inline QInputEvent(Type type) : QEvent(type), accpt(true){}
    inline bool isAccepted() const { return accpt; }
    inline void accept() { accpt = TRUE; }
    inline void ignore() { accpt = FALSE; }
private:
    bool accpt;
};


class Q_GUI_EXPORT QMouseEvent : public QInputEvent
{
public:
    QMouseEvent(Type type, const QPoint &pos, Qt::MouseButton button, 
                Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);
    inline QMouseEvent(Type type, const QPoint &pos, const QPoint &globalPos,
                       Qt::MouseButton button, Qt::MouseButtons buttons, 
                       Qt::KeyboardModifiers modifiers)
        : QInputEvent(type), p(pos), g(globalPos), b(button), mouseState(buttons), keyState(modifiers) {}
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QMouseEvent(Type type, const QPoint &pos, Qt::ButtonState button, int state);
    QT_COMPAT_CONSTRUCTOR QMouseEvent(Type type, const QPoint &pos, const QPoint &globalPos, 
                                      Qt::ButtonState button, int state);
#endif

    inline const QPoint &pos() const { return p; }
    inline const QPoint &globalPos() const { return g; }
    inline int x() const { return p.x(); }
    inline int y() const { return p.y(); }
    inline int globalX() const { return g.x(); }
    inline int globalY() const { return g.y(); }
#ifdef QT_COMPAT
    inline QT_COMPAT Qt::ButtonState state() const 
    { return Qt::ButtonState(int(mouseState)|int(keyState)); }
    inline QT_COMPAT Qt::ButtonState stateAfter() const 
    { return Qt::ButtonState(int(buttons())|int(modifiers())); }
#endif
    inline Qt::MouseButton button() const { return b; }
    inline Qt::MouseButtons buttons() const { return mouseState^b; }
    inline Qt::KeyboardModifiers modifiers() const { return keyState; }

protected:
    QPoint p, g;
    Qt::MouseButton b;
    Qt::MouseButtons mouseState;
    Qt::KeyboardModifiers keyState;
};


#ifndef QT_NO_WHEELEVENT
class Q_GUI_EXPORT QWheelEvent : public QInputEvent
{
public:
    QWheelEvent(const QPoint &pos, int delta, 
                Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers,
                Qt::Orientation orient = Qt::Vertical);
    inline QWheelEvent(const QPoint &pos, const QPoint& globalPos, int delta, 
                       Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers,
                       Qt::Orientation orient = Qt::Vertical)
        : QInputEvent(Wheel), p(pos), g(globalPos), d(delta), mouseState(buttons), 
          keyState(modifiers),o(orient) {}
#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QWheelEvent(const QPoint &pos, int delta, int state, 
                                      Qt::Orientation orient = Qt::Vertical);
    QT_COMPAT_CONSTRUCTOR QWheelEvent(const QPoint &pos, const QPoint& globalPos, int delta, int state, 
                                      Qt::Orientation orient = Qt::Vertical);
#endif
    inline int delta() const { return d; }
    inline const QPoint &pos() const { return p; }
    inline const QPoint &globalPos()   const { return g; }
    inline int x() const { return p.x(); }
    inline int y() const { return p.y(); }
    inline int globalX() const { return g.x(); }
    inline int globalY() const { return g.y(); }
#ifdef QT_COMPAT
    inline QT_COMPAT Qt::ButtonState state() const 
    { return static_cast<Qt::ButtonState>(int(buttons())|int(modifiers())); }
#endif
    inline Qt::MouseButtons buttons() const { return mouseState; }
    inline Qt::KeyboardModifiers modifiers() const { return keyState; }

    Qt::Orientation orientation() const { return o; }
protected:
    QPoint p;
    QPoint g;
    int d;
    Qt::MouseButtons mouseState;
    Qt::KeyboardModifiers keyState;
    Qt::Orientation o;
};
#endif

class Q_GUI_EXPORT QTabletEvent : public QInputEvent
{
public:
    enum TabletDevice { NoDevice = -1, Puck, Stylus, Eraser };
    QTabletEvent(Type t, const QPoint &pos, const QPoint &globalPos, const QPoint &hiResPos, 
                  int minX, int maxX, int minY, int maxY, int device,
                  int pressure, int minPressure, int maxPressure, int xTilt, int yTilt,
                  const QPair<int,int> &uId);
    inline int pressure() const { return mPress; }
    inline int xTilt() const { return mXT; }
    inline int yTilt() const { return mYT; }
    inline const QPoint &pos() const { return mPos; }
    inline const QPoint &globalPos() const { return mGPos; }
    inline const QPoint &hiResPos() const { return mHiResPos; }
    inline int x() const { return mPos.x(); }
    inline int y() const { return mPos.y(); }
    inline int globalX() const { return mGPos.x(); }
    inline int globalY() const { return mGPos.y(); }
    inline int hiResX() const { return mHiResPos.x(); }
    inline int hiResY() const { return mHiResPos.y(); }
    inline TabletDevice device() const { return TabletDevice(mDev); }
    inline QPair<int, int> uniqueId() { return QPair<int,int>(mType, mPhy); }
    inline int minPressure() const { return mMinPressure; }
    inline int maxPressure() const { return mMaxPressure; }
    inline int minHiResX() const { return mHiResMinX; }
    inline int minHiResY() const { return mHiResMinY; }
    inline int maxHiResX() const { return mHiResMaxX; }
    inline int maxHiResY() const { return mHiResMaxY; }
        
protected:
    QPoint mPos, mGPos, mHiResPos;
    int mHiResMinX, mHiResMaxX, mHiResMinY, mHiResMaxY;
    int mDev, mPress, mXT, mYT, mType, mPhy;
    int mMinPressure, mMaxPressure;
};


class Q_GUI_EXPORT QKeyEvent : public QInputEvent
{
public:
    inline QKeyEvent(Type type, int key, Qt::KeyboardModifiers modifiers, 
                     const QString& text = QString::null, 
                     bool autorep = FALSE, ushort count = 1)
        : QInputEvent(type), txt(text), k(key), m(modifiers), c(count), autor(autorep)
    {
        if(key >= Qt::Key_Back && key <= Qt::Key_MediaLast)
            ignore();
    }
    int key() const   { return k; }
#ifdef QT_COMPAT
    inline QT_COMPAT_CONSTRUCTOR QKeyEvent(Type type, int key, int modifiers, 
                                           const QString& text = QString::null, 
                                           bool autorep = FALSE, ushort count = 1)
        : QInputEvent(type), txt(text), k(key), m((Qt::KeyboardModifiers)modifiers), 
          c(count), autor(autorep)
    {
        if(key >= Qt::Key_Back && key <= Qt::Key_MediaLast)
            ignore();
    }
    inline QT_COMPAT_CONSTRUCTOR QKeyEvent(Type type, int key, int /*ascii*/, 
                                           int modifiers, const QString& text = QString::null,
                                           bool autorep = FALSE, ushort count = 1)
        : QInputEvent(type), txt(text), k(key), m((Qt::KeyboardModifiers)modifiers), 
          c(count), autor(autorep)
    {
        if (key >= Qt::Key_Back && key <= Qt::Key_MediaLast)
            ignore();
    }
    inline QT_COMPAT int ascii() const 
    { return (txt.length() ? txt.unicode()->latin1() : 0); }
    inline QT_COMPAT Qt::ButtonState state() const { return Qt::ButtonState(m); }
    inline QT_COMPAT Qt::ButtonState stateAfter() const { return Qt::ButtonState(modifiers()); }
#endif
    Qt::KeyboardModifiers modifiers() const;
    inline QString text() const { return txt; }
    inline bool isAutoRepeat() const { return autor; }
    inline int count() const { return int(c); }

protected:
    QString txt;
    int k;
    Qt::KeyboardModifiers m;
    ushort c;
    uint autor:1;
};


class Q_GUI_EXPORT QFocusEvent : public QEvent
{
public:
    inline QFocusEvent(Type type) : QEvent(type) {}

    inline bool gotFocus() const { return type() == FocusIn; }
    inline bool lostFocus() const { return type() == FocusOut; }

    enum Reason { Mouse, Tab, Backtab, ActiveWindow, Popup, Shortcut, Other };
    static Reason reason();
    static void setReason(Reason reason);
    static void resetReason();

private:
    static Reason m_reason;
    static Reason prev_reason;
};


class Q_GUI_EXPORT QPaintEvent : public QEvent
{
public:
    inline QPaintEvent(const QRegion& paintRegion)
        : QEvent(Paint), rec(paintRegion.boundingRect()), reg(paintRegion){}
    inline QPaintEvent(const QRect &paintRect)
        : QEvent(Paint), rec(paintRect),reg(paintRect){}
    inline QPaintEvent(const QRegion &paintRegion, const QRect &paintRect)
        : QEvent(Paint), rec(paintRect), reg(paintRegion){}

    inline const QRect &rect() const { return rec; }
    inline const QRegion &region() const { return reg; }

#ifdef QT_COMPAT
    inline QT_COMPAT bool erased() const { return true; }
#endif

protected:
    friend class QApplication;
    friend class QCoreApplication;
    QRect rec;
    QRegion reg;
};


#ifdef Q_WS_QWS
class QWSUpdateEvent : public QPaintEvent
{
public:
    inline QWSUpdateEvent(const QRegion& paintRegion)
        : QPaintEvent(paintRegion) { t = QWSUpdate; }
    inline QWSUpdateEvent(const QRect &paintRect)
        : QPaintEvent(paintRect) { t = QWSUpdate; }
};
#endif


class Q_GUI_EXPORT QMoveEvent : public QEvent
{
public:
    inline QMoveEvent(const QPoint &pos, const QPoint &oldPos): 
        QEvent(Move), p(pos), oldp(oldPos) {}
    inline const QPoint &pos() const { return p; }
    inline const QPoint &oldPos() const { return oldp;}
protected:
    QPoint p, oldp;
    friend class QApplication;
    friend class QCoreApplication;
};


class Q_GUI_EXPORT QResizeEvent : public QEvent
{
public:
    inline QResizeEvent(const QSize &size, const QSize &oldSize)
        : QEvent(Resize), s(size), olds(oldSize) {}
    inline const QSize &size() const { return s; }
    inline const QSize &oldSize()const { return olds;}
protected:
    QSize s, olds;
    friend class QApplication;
    friend class QCoreApplication;
};


class Q_GUI_EXPORT QCloseEvent : public QInputEvent
{
public:
    inline QCloseEvent() : QInputEvent(Close) {}
};


class Q_GUI_EXPORT QIconDragEvent : public QEvent
{
public:
    inline QIconDragEvent() : QEvent(IconDrag), accpt(FALSE) {}
    inline bool isAccepted() const { return accpt; }
    inline void accept() { accpt = TRUE; }
    inline void ignore() { accpt = FALSE; }
protected:
    bool accpt;
};


class Q_GUI_EXPORT QShowEvent : public QEvent
{
public:
    inline QShowEvent() : QEvent(Show) {}
};


class Q_GUI_EXPORT QHideEvent : public QEvent
{
public:
    inline QHideEvent() : QEvent(Hide) {}
};


class Q_GUI_EXPORT QContextMenuEvent : public QInputEvent
{
public:
    enum Reason { Mouse, Keyboard, Other };
    inline QContextMenuEvent(Reason reason, const QPoint &pos, const QPoint &globalPos)
        : QInputEvent(ContextMenu), p(pos), gp(globalPos), reas(reason) {}
    QContextMenuEvent(Reason reason, const QPoint &pos);
#ifdef QT_COMPAT
    inline QT_COMPAT_CONSTRUCTOR QContextMenuEvent(Reason reason, const QPoint &pos, 
                                                   const QPoint &globalPos, int)
        : QInputEvent(ContextMenu), p(pos), gp(globalPos), reas(reason) {}
    QT_COMPAT_CONSTRUCTOR QContextMenuEvent(Reason reason, const QPoint &pos, int);
#endif

    inline int x() const { return p.x(); }
    inline int y() const { return p.y(); }
    inline int globalX() const { return gp.x(); }
    inline int globalY() const { return gp.y(); }

    inline const QPoint& pos() const { return p; }
    inline const QPoint& globalPos() const { return gp; }

#ifdef QT_COMPAT
    QT_COMPAT Qt::ButtonState state() const;
#endif
    inline Reason reason() const { return Reason(reas); }

protected:
    QPoint p;
    QPoint gp;
    uint reas : 8;
};


class Q_GUI_EXPORT QIMEvent : public QInputEvent
{
public:
    inline QIMEvent(Type type, const QString &text, int cursorPosition, int selLength = 0)
        : QInputEvent(type), txt(text), cpos(cursorPosition), selLen(selLength) {}
    inline const QString &text() const { return txt; }
    inline int cursorPos() const { return cpos; }
    inline int selectionLength() const { return selLen; }

private:
    QString txt;
    int cpos;
    int selLen;
};


#ifndef QT_NO_DRAGANDDROP
// This class is rather closed at the moment.  If you need to create your
// own DND event objects, write to qt-bugs@trolltech.com and we'll try to
// find a way to extend it so it covers your needs.
class Q_GUI_EXPORT QDropEvent : public QEvent, public QMimeSource
{
public:
    inline QDropEvent(const QPoint& pos, Type typ = Drop)
        : QEvent(typ), p(pos), act(0), accpt(0), accptact(0), resv(0), d(0){}
    inline const QPoint &pos() const { return p; }
    inline bool isAccepted() const { return accpt || accptact; }
    inline void accept(bool y = TRUE) { accpt = y; }
    inline void ignore() { accpt = FALSE; }

    enum Action { Copy, Link, Move, Private, UserAction = 100 };
    inline bool isActionAccepted() const { return accptact; }
    inline void acceptAction(bool y = TRUE)  { accptact = y; }
    inline void setAction(Action a) { act = uint(a); }
    inline Action action() const { return Action(act); }

    QWidget* source() const;
    const char* format(int n = 0) const;
    QByteArray encodedData(const char*) const;
    bool provides(const char*) const;

    inline QByteArray data(const char* f) const { return encodedData(f); }

    inline void setPoint(const QPoint& np) { p = np; }

protected:
    QPoint p;
    uint act : 8;
    uint accpt : 1;
    uint accptact : 1;
    uint resv : 5;
    void *d;
};


class Q_GUI_EXPORT QDragMoveEvent : public QDropEvent
{
public:
    inline QDragMoveEvent(const QPoint& pos, Type typ = DragMove)
        : QDropEvent(pos,typ), rect(pos, QSize(1, 1)) {}
    inline QRect answerRect() const { return rect; }
    inline void accept(bool y = true) { QDropEvent::accept(y); }
    inline void ignore() { QDropEvent::ignore(); }

    inline void accept(const QRect & r) { accpt = TRUE; rect = r; }
    inline void ignore(const QRect & r) { accpt = FALSE; rect = r; }

protected:
    QRect rect;
};


class Q_GUI_EXPORT QDragEnterEvent : public QDragMoveEvent
{
public:
    inline QDragEnterEvent(const QPoint& pos) : QDragMoveEvent(pos, DragEnter) { }
};


/* An internal class */
class Q_GUI_EXPORT QDragResponseEvent : public QEvent
{
public:
    inline QDragResponseEvent(bool accepted)
        : QEvent(DragResponse), a(accepted) {}
    inline bool dragAccepted() const { return a; }
protected:
    bool a;
};


class Q_GUI_EXPORT QDragLeaveEvent : public QEvent
{
public:
    inline QDragLeaveEvent() : QEvent(DragLeave) {}
};
#endif // QT_NO_DRAGANDDROP


class Q_GUI_EXPORT QHelpEvent : public QEvent
{
public:
    inline QHelpEvent(Type type, const QPoint &pos, const QPoint &globalPos)
        : QEvent(type), p(pos), gp(globalPos) {}

    inline int x() const { return p.x(); }
    inline int y() const { return p.y(); }
    inline int globalX() const { return gp.x(); }
    inline int globalY() const { return gp.y(); }

    inline const QPoint& pos()  const { return p; }
    inline const QPoint& globalPos() const { return gp; }

private:
    QPoint p;
    QPoint gp;
};


class Q_GUI_EXPORT QStatusTipEvent : public QEvent
{
public:
    inline QStatusTipEvent(const QString &tip):QEvent(StatusTip), s(tip){}
    inline QString tip() const { return s; }
private:
    QString s;
};

class Q_GUI_EXPORT QWhatsThisClickedEvent : public QEvent
{
public:
    inline QWhatsThisClickedEvent(const QString &href)
        : QEvent(WhatsThisClicked), s(href){}
    inline QString href() const { return s; }
private:
    QString s;
};


class Q_GUI_EXPORT QActionEvent : public QEvent
{
    QAction *act, *bef;
public:
    inline QActionEvent(int type, QAction *action, QAction *before = 0)
        : QEvent(static_cast<QEvent::Type>(type)), act(action), bef(before) { }

    inline QAction *action() const { return act; }
    inline QAction *before() const { return bef; }
};


class Q_GUI_EXPORT QFileOpenEvent : public QEvent
{
public:
    inline QFileOpenEvent(const QString &file) : QEvent(FileOpen), f(file) { }
    inline QString file() const { return f; }
private:
    QString f;
};

class Q_GUI_EXPORT QToolBarChangeEvent : public QEvent
{
public:
    inline QToolBarChangeEvent(bool t) : QEvent(ToolBarChange), tog(t) {}
    inline bool toggle() const { return tog; }
private:
    uint tog : 1;
};

class Q_GUI_EXPORT QShortcutEvent : public QEvent
{
public:
    inline QShortcutEvent(const QKeySequence &key, int id, bool ambiguous = false)
	: QEvent(Shortcut), sequence(key), ambig(ambiguous), sid(id) { }
    inline const QKeySequence &key() { return sequence; }
    inline int shortcutId() { return sid; }
    inline bool isAmbiguous() { return ambig; }
protected:
    QKeySequence sequence;
    bool ambig;
    int  sid;
};

#ifndef QT_NO_DEBUG
Q_GUI_EXPORT QDebug operator<<(QDebug, const QEvent *);
#endif

#endif // QEVENT_H
