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

#include "QtGui/qwindowdefs.h"
#include "QtCore/qobject.h"
#include "QtGui/qregion.h"
#include "QtCore/qnamespace.h"
#include "QtCore/qstring.h"
#include "QtGui/qkeysequence.h"
#include "QtCore/qcoreevent.h"
#include "QtGui/qmime.h"
#include "QtGui/qdrag.h"

class QAction;

class Q_GUI_EXPORT QInputEvent : public QEvent
{
public:
    QInputEvent(Type type, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
    ~QInputEvent();
    inline Qt::KeyboardModifiers modifiers() const {return modState; }
protected:
    Qt::KeyboardModifiers modState;
};


class Q_GUI_EXPORT QMouseEvent : public QInputEvent
{
public:
    QMouseEvent(Type type, const QPoint &pos, Qt::MouseButton button,
                Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);
    QMouseEvent(Type type, const QPoint &pos, const QPoint &globalPos,
                Qt::MouseButton button, Qt::MouseButtons buttons,
                Qt::KeyboardModifiers modifiers);
    ~QMouseEvent();

    inline const QPoint &pos() const { return p; }
    inline const QPoint &globalPos() const { return g; }
    inline int x() const { return p.x(); }
    inline int y() const { return p.y(); }
    inline int globalX() const { return g.x(); }
    inline int globalY() const { return g.y(); }
    inline Qt::MouseButton button() const { return b; }
    inline Qt::MouseButtons buttons() const { return mouseState; }

#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QMouseEvent(Type type, const QPoint &pos, Qt::ButtonState button, int state);
    QT_COMPAT_CONSTRUCTOR QMouseEvent(Type type, const QPoint &pos, const QPoint &globalPos,
                                      Qt::ButtonState button, int state);
    inline QT_COMPAT Qt::ButtonState state() const
    { return Qt::ButtonState((mouseState^b)|int(modifiers())); }
    inline QT_COMPAT Qt::ButtonState stateAfter() const
    { return Qt::ButtonState(int(mouseState)|int(modifiers())); }
#endif
protected:
    QPoint p, g;
    Qt::MouseButton b;
    Qt::MouseButtons mouseState;
};


#ifndef QT_NO_WHEELEVENT
class Q_GUI_EXPORT QWheelEvent : public QInputEvent
{
public:
    QWheelEvent(const QPoint &pos, int delta,
                Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers,
                Qt::Orientation orient = Qt::Vertical);
    QWheelEvent(const QPoint &pos, const QPoint& globalPos, int delta,
                Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers,
                Qt::Orientation orient = Qt::Vertical);
    ~QWheelEvent();

    inline int delta() const { return d; }
    inline const QPoint &pos() const { return p; }
    inline const QPoint &globalPos()   const { return g; }
    inline int x() const { return p.x(); }
    inline int y() const { return p.y(); }
    inline int globalX() const { return g.x(); }
    inline int globalY() const { return g.y(); }

    inline Qt::MouseButtons buttons() const { return mouseState; }
    Qt::Orientation orientation() const { return o; }

#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QWheelEvent(const QPoint &pos, int delta, int state,
                                      Qt::Orientation orient = Qt::Vertical);
    QT_COMPAT_CONSTRUCTOR QWheelEvent(const QPoint &pos, const QPoint& globalPos, int delta, int state,
                                      Qt::Orientation orient = Qt::Vertical);
    inline QT_COMPAT Qt::ButtonState state() const
    { return static_cast<Qt::ButtonState>(int(buttons())|int(modifiers())); }
#endif
protected:
    QPoint p;
    QPoint g;
    int d;
    Qt::MouseButtons mouseState;
    Qt::Orientation o;
};
#endif

class Q_GUI_EXPORT QTabletEvent : public QInputEvent
{
public:
    enum TabletDevice { NoDevice = -1, Puck, Stylus, Eraser };
    QTabletEvent(Type t, const QPoint &pos,  const QPoint &globalPos, const QPointF &hiResGlobalPos,
                 int device, qReal pressure, int xTilt, int yTilt,
                 Qt::KeyboardModifiers keyState, Q_LONGLONG uniqueID);
    ~QTabletEvent();

    inline qReal pressure() const { return mPress; }
    inline int xTilt() const { return mXT; }
    inline int yTilt() const { return mYT; }
    inline const QPoint &pos() const { return mPos; }
    inline const QPoint &globalPos() const { return mGPos; }
    inline const QPointF &hiResGlobalPos() const { return mHiResGlobalPos; }
    inline int x() const { return mPos.x(); }
    inline int y() const { return mPos.y(); }
    inline int globalX() const { return mGPos.x(); }
    inline int globalY() const { return mGPos.y(); }
    inline qReal hiResGlobalX() const { return mHiResGlobalPos.x(); }
    inline qReal hiResGlobalY() const { return mHiResGlobalPos.y(); }
    inline TabletDevice device() const { return TabletDevice(mDev); }
    inline Q_LONGLONG uniqueId() const { return mUnique; }

protected:
    QPoint mPos, mGPos;
    QPointF mHiResGlobalPos;
    int mDev, mXT, mYT;
    qReal mPress;
    Q_LONGLONG mUnique;
};


class Q_GUI_EXPORT QKeyEvent : public QInputEvent
{
public:
    QKeyEvent(Type type, int key, Qt::KeyboardModifiers modifiers,
              const QString& text = QString::null,
              bool autorep = false, ushort count = 1);
    ~QKeyEvent();

    int key() const   { return k; }
    Qt::KeyboardModifiers modifiers() const;
    inline QString text() const { return txt; }
    inline bool isAutoRepeat() const { return autor; }
    inline int count() const { return int(c); }

#ifdef QT_COMPAT
    inline QT_COMPAT_CONSTRUCTOR QKeyEvent(Type type, int key, int /*ascii*/,
                                           int modifiers, const QString& text = QString::null,
                                           bool autorep = false, ushort count = 1)
        : QInputEvent(type, (Qt::KeyboardModifiers)(modifiers & Qt::KeyButtonMask)), txt(text), k(key),
          c(count), autor(autorep)
    {
        if (key >= Qt::Key_Back && key <= Qt::Key_MediaLast)
            ignore();
    }
    inline QT_COMPAT int ascii() const
    { return (txt.length() ? txt.unicode()->toLatin1() : 0); }
    inline QT_COMPAT Qt::ButtonState state() const { return Qt::ButtonState(QInputEvent::modifiers()); }
    inline QT_COMPAT Qt::ButtonState stateAfter() const { return Qt::ButtonState(modifiers()); }
#endif
protected:
    QString txt;
    int k;
    ushort c;
    uint autor:1;
};


class Q_GUI_EXPORT QFocusEvent : public QEvent
{
public:
    QFocusEvent(Type type);
    ~QFocusEvent();

    inline bool gotFocus() const { return type() == FocusIn; }
    inline bool lostFocus() const { return type() == FocusOut; }

    enum Reason { Mouse, Tab, Backtab, ActiveWindow, Popup, Shortcut, MenuBar, Other };
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
    QPaintEvent(const QRegion& paintRegion);
    QPaintEvent(const QRect &paintRect);
    QPaintEvent(const QRegion &paintRegion, const QRect &paintRect);
    ~QPaintEvent();

    inline const QRect &rect() const { return m_rect; }
    inline const QRegion &region() const { return m_region; }

#ifdef QT_COMPAT
    inline QT_COMPAT bool erased() const { return true; }
#endif

protected:
    friend class QApplication;
    friend class QCoreApplication;
    QRect m_rect;
    QRegion m_region;
};


#ifdef Q_WS_QWS
class QWSUpdateEvent : public QPaintEvent
{
public:
    QWSUpdateEvent(const QRegion& paintRegion);
    QWSUpdateEvent(const QRect &paintRect);
    ~QWSUpdateEvent();
};
#endif


class Q_GUI_EXPORT QMoveEvent : public QEvent
{
public:
    QMoveEvent(const QPoint &pos, const QPoint &oldPos);
    ~QMoveEvent();

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
    QResizeEvent(const QSize &size, const QSize &oldSize);
    ~QResizeEvent();

    inline const QSize &size() const { return s; }
    inline const QSize &oldSize()const { return olds;}
protected:
    QSize s, olds;
    friend class QApplication;
    friend class QCoreApplication;
};


class Q_GUI_EXPORT QCloseEvent : public QEvent
{
public:
    QCloseEvent();
    ~QCloseEvent();
};


class Q_GUI_EXPORT QIconDragEvent : public QEvent
{
public:
    QIconDragEvent();
    ~QIconDragEvent();
};


class Q_GUI_EXPORT QShowEvent : public QEvent
{
public:
    QShowEvent();
    ~QShowEvent();
};


class Q_GUI_EXPORT QHideEvent : public QEvent
{
public:
    QHideEvent();
    ~QHideEvent();
};


class Q_GUI_EXPORT QContextMenuEvent : public QInputEvent
{
public:
    enum Reason { Mouse, Keyboard, Other };

    QContextMenuEvent(Reason reason, const QPoint &pos, const QPoint &globalPos);
    QContextMenuEvent(Reason reason, const QPoint &pos);
    ~QContextMenuEvent();

    inline int x() const { return p.x(); }
    inline int y() const { return p.y(); }
    inline int globalX() const { return gp.x(); }
    inline int globalY() const { return gp.y(); }

    inline const QPoint& pos() const { return p; }
    inline const QPoint& globalPos() const { return gp; }

    inline Reason reason() const { return Reason(reas); }

#ifdef QT_COMPAT
    QT_COMPAT_CONSTRUCTOR QContextMenuEvent(Reason reason, const QPoint &pos, const QPoint &globalPos, int);
    QT_COMPAT_CONSTRUCTOR QContextMenuEvent(Reason reason, const QPoint &pos, int);

    QT_COMPAT Qt::ButtonState state() const;
#endif
protected:
    QPoint p;
    QPoint gp;
    uint reas : 8;
};

#if 0
class Q_GUI_EXPORT QInputMethodEvent : public QInputEvent
{
public:
    QInputMethodEvent(const QString &text);

    enum AttributeType {
       UnderLineColor,
       BackgroundColor,
       CursorPosition,
       ReplaceString,
       Language,
       RubyText
    };
    struct Attribute {
        AttributeType type;
        QVariant value;
        int start;
        int length;
    };
    void setAttribute(const Attribute &value);
    QVariant attribute(AttributeType a) const;
    QList<Attribute> attributes() const;

private:
    QList<Attribute> attributes;
    QString preedit;
};
#else
class Q_GUI_EXPORT QInputMethodEvent : public QInputEvent
{
public:
    QInputMethodEvent(Type type, const QString &text, int cursorPosition, int selLength = 0);
    ~QInputMethodEvent();

    inline const QString &text() const { return txt; }
    inline int cursorPos() const { return cpos; }
    inline int selectionLength() const { return selLen; }

private:
    QString txt;
    int cpos;
    int selLen;
};
#endif

#ifndef QT_NO_DRAGANDDROP

class QMimeData;

class Q_GUI_EXPORT QDropEvent : public QEvent
// QT_COMPAT
                              , public QMimeSource
// END QT_COMPAT
{
public:
    QDropEvent(const QPoint& pos, QDrag::DropActions actions, const QMimeData *data, Type type = Drop);
    ~QDropEvent();

    inline const QPoint &pos() const { return p; }

    inline QDrag::DropActions possibleActions() const { return act; }
    inline QDrag::DropAction proposedAction() const { return default_action; }
    inline void acceptProposedAction() { drop_action = default_action; accept(); }

    inline QDrag::DropAction dropAction() const { return drop_action; }
    void setDropAction(QDrag::DropAction action);

    QWidget* source() const;
    inline const QMimeData *mimeData() const { return mdata; }

// QT_COMPAT
    const char* format(int n = 0) const;
    QByteArray encodedData(const char*) const;
    bool provides(const char*) const;
// END QT_COMPAT
#ifdef QT_COMPAT
    inline void accept() { QEvent::accept(); }
    inline QT_COMPAT void accept(bool y) { setAccepted(y); }
    inline QT_COMPAT QByteArray data(const char* f) const { return encodedData(f); }

    enum Action { Copy, Link, Move, Private, UserAction = Private };
    QT_COMPAT Action action() const;
    inline QT_COMPAT void acceptAction(bool y = true)  { if (y) { drop_action = default_action; accept(); } }
    inline QT_COMPAT void setPoint(const QPoint& np) { p = np; }
#endif


protected:
    QPoint p;
    QDrag::DropActions act;
    QDrag::DropAction drop_action;
    QDrag::DropAction default_action;
    const QMimeData *mdata;
    mutable QList<QByteArray> fmts; // only used for QT_COMPAT
};


class Q_GUI_EXPORT QDragMoveEvent : public QDropEvent
{
public:
    QDragMoveEvent(const QPoint &pos, QDrag::DropActions actions, const QMimeData *data, Type typ = DragMove);
    ~QDragMoveEvent();

    inline QRect answerRect() const { return rect; }

    inline void accept() { QDropEvent::accept(); }
    inline void ignore() { QDropEvent::ignore(); }

    inline void accept(const QRect & r) { accept(); rect = r; }
    inline void ignore(const QRect & r) { ignore(); rect = r; }

#ifdef QT_COMPAT
    inline QT_COMPAT void accept(bool y) { setAccepted(y); }
#endif

protected:
    QRect rect;
};


class Q_GUI_EXPORT QDragEnterEvent : public QDragMoveEvent
{
public:
    QDragEnterEvent(const QPoint &pos, QDrag::DropActions actions, const QMimeData *data);
    ~QDragEnterEvent();
};


/* An internal class */
class Q_GUI_EXPORT QDragResponseEvent : public QEvent
{
public:
    QDragResponseEvent(bool accepted);
    ~QDragResponseEvent();

    inline bool dragAccepted() const { return a; }
protected:
    bool a;
};


class Q_GUI_EXPORT QDragLeaveEvent : public QEvent
{
public:
    QDragLeaveEvent();
    ~QDragLeaveEvent();
};
#endif // QT_NO_DRAGANDDROP


class Q_GUI_EXPORT QHelpEvent : public QEvent
{
public:
    QHelpEvent(Type type, const QPoint &pos, const QPoint &globalPos);
    ~QHelpEvent();

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
    QStatusTipEvent(const QString &tip);
    ~QStatusTipEvent();

    inline QString tip() const { return s; }
private:
    QString s;
};

class Q_GUI_EXPORT QWhatsThisClickedEvent : public QEvent
{
public:
    QWhatsThisClickedEvent(const QString &href);
    ~QWhatsThisClickedEvent();

    inline QString href() const { return s; }
private:
    QString s;
};


class Q_GUI_EXPORT QActionEvent : public QEvent
{
    QAction *act, *bef;
public:
    QActionEvent(int type, QAction *action, QAction *before = 0);
    ~QActionEvent();

    inline QAction *action() const { return act; }
    inline QAction *before() const { return bef; }
};


class Q_GUI_EXPORT QFileOpenEvent : public QEvent
{
public:
    QFileOpenEvent(const QString &file);
    ~QFileOpenEvent();

    inline QString file() const { return f; }
private:
    QString f;
};

class Q_GUI_EXPORT QToolBarChangeEvent : public QEvent
{
public:
    QToolBarChangeEvent(bool t);
    ~QToolBarChangeEvent();

    inline bool toggle() const { return tog; }
private:
    uint tog : 1;
};

class Q_GUI_EXPORT QShortcutEvent : public QEvent
{
public:
    QShortcutEvent(const QKeySequence &key, int id, bool ambiguous = false);
    ~QShortcutEvent();

    inline const QKeySequence &key() { return sequence; }
    inline int shortcutId() { return sid; }
    inline bool isAmbiguous() { return ambig; }
protected:
    QKeySequence sequence;
    bool ambig;
    int  sid;
};

class Q_GUI_EXPORT QClipboardEvent : public QEvent
{
public:
    QClipboardEvent(QEventPrivate *data);
    ~QClipboardEvent();

    QEventPrivate *data() { return d; };
};

#ifndef QT_NO_DEBUG_OUTPUT
Q_GUI_EXPORT QDebug operator<<(QDebug, const QEvent *);
#endif

#endif // QEVENT_H
