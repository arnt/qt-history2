/****************************************************************************
**
** Definition of QCursor class.
**
** Copyright (C) 1992-$THISYEAR$ Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCURSOR_H
#define QCURSOR_H

#ifndef QT_H
#include <qpoint.h>
#include <qatomic.h>
#endif // QT_H

/*
  ### The fake cursor has to go first with old qdoc.
*/
#ifdef QT_NO_CURSOR

class Q_GUI_EXPORT QCursor : public Qt
{
public:
    static QPoint pos();
    static void          setPos(int x, int y);
    static void          setPos(const QPoint &);
private:
    QCursor();
};

#endif // QT_NO_CURSOR

#ifndef QT_NO_CURSOR

struct QCursorData;


class Q_GUI_EXPORT QCursor
{
public:
    QCursor();
    QCursor(int shape);
    QCursor(const QBitmap &bitmap, const QBitmap &mask, int hotX=-1, int hotY=-1);
    QCursor(const QPixmap &pixmap, int hotX=-1, int hotY=-1);
    QCursor(const QCursor &cursor);
    ~QCursor();
    QCursor &operator=(const QCursor &cursor);

    int shape() const;
    void setShape(int newShape);

    const QBitmap *bitmap() const;
    const QBitmap *mask() const;
    QPoint hotSpot() const;
#if defined(Q_WS_WIN)
    HCURSOR handle()  const;
    QCursor(HCURSOR cursor);
#elif defined(Q_WS_X11)
    Qt::HANDLE handle() const;
    QCursor(Qt::HANDLE cursor);
    static int x11Screen();
#elif defined(Q_WS_MAC)
    HANDLE handle() const;
#elif defined(Q_WS_QWS)
    HANDLE handle() const;
#endif
    static QPoint pos();
    static void        setPos(int x, int y);
    inline static void setPos(const QPoint &p) { setPos(p.x(), p.y()); }
    static void        initialize();
    static void        cleanup();
private:
    void setBitmap(const QBitmap &bitmap, const QBitmap &mask, int hotX, int hotY);
    void update() const;
    QCursor *find_cur(int);

    QCursorData *d;
    static bool initialized;
#if defined(Q_WS_MAC)
    friend void qt_mac_set_cursor(const QCursor *c, const Point *p);
#endif
};

#ifdef QT_COMPAT
// CursorShape is defined in X11/X.h
#ifdef CursorShape
#define X_CursorShape CursorShape
#undef CursorShape
#endif
typedef Qt::CursorShape QCursorShape;
#ifdef X_CursorShape
#define CursorShape X_CursorShape
#endif
#endif

/*****************************************************************************
  QCursor stream functions
 *****************************************************************************/
#ifndef QT_NO_DATASTREAM
Q_GUI_EXPORT QDataStream &operator<<(QDataStream &outS, const QCursor &cursor);
Q_GUI_EXPORT QDataStream &operator>>(QDataStream &inS, QCursor &cursor);
#endif
#endif // QT_NO_CURSOR

#endif // QCURSOR_H
