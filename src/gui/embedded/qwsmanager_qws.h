/****************************************************************************
**
** Definition of QWSManager class. This manages QWS top-level windows.
**
** Copyright (C) 1992-2003 Trolltech AS. All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
** EDITIONS: FREE, PROFESSIONAL, ENTERPRISE
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QWSMANAGER_QWS_H
#define QWSMANAGER_QWS_H

#ifndef QT_H
#include "qpixmap.h"
#include "qobject.h"
#include "qwsdecoration_qws.h"
#include "qevent.h"
#endif // QT_H

#ifndef QT_NO_QWS_MANAGER

class QPixmap;
class QWidget;
class QPopupMenu;
class QRegion;
class QMouseEvent;
class QWSButton;
class QWSManager;

class QWSManager : public QObject
{
    Q_OBJECT

public:
    QWSManager(QWidget *);
    ~QWSManager();

    static QWSDecoration *newDefaultDecoration();

    QRegion region();
    QWidget *widget() { return managed; }
    void maximize();

    static QWidget *grabbedMouse() { return active; }

protected slots:
    void menuActivated(int);
    void styleMenuActivated(int);

protected:
    void handleMove();
    virtual QWSDecoration::Region pointInRegion(const QPoint &);

    virtual bool event(QEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *);
    virtual void mousePressEvent(QMouseEvent *);
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void mouseDoubleClickEvent(QMouseEvent *) {}
    virtual void paintEvent(QPaintEvent *);

    void menu(const QPoint &);
    void close();
    void minimize();
    void toggleMaximize();

    QWSDecoration::Region activeRegion;
    QWidget *managed;
    QPopupMenu *popup;
    QRect   normalSize;
    QWSButton *menuBtn;
    QWSButton *closeBtn;
    QWSButton *minimizeBtn;
    QWSButton *maximizeBtn;

    int dx;
    int dy;
    int skipCount;

    static QWidget *active;
    static QPoint mousePos;

private:
    void setMouseOver(QWSButton *, bool);
    void setClicked(QWSButton *, bool);
    void setOn(QWSButton *, bool);
    void repaintButton(QWSButton *);


};

class QWSButton
{
public:
    QWSButton(QWSManager *m, QWSDecoration::Region t, bool tb = false);

    enum State { MouseOver = 0x01, Clicked = 0x02, On = 0x04 };
    int state() { return flags; }
    QWSDecoration::Region type() { return typ; }
    bool setMouseOver(bool);
    bool setClicked(bool);
    bool setOn(bool);

private:
    int  flags;
    bool toggle;
    QWSDecoration::Region typ;
    QWSManager *manager;
};

// class QWSDefaultDecoration : public QWSDecoration;
#include "qwsdefaultdecoration_qws.h"

#endif // QT_NO_QWS_MANAGER

#endif // QWSMANAGER_QWS_H
