/****************************************************************************
** $Id: //depot/qt/main/src/kernel/qwsmanager_qws.h $
**
** Definition of QWSManager class. This manages QWS top-level windows.
**
** Created : 20000308
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef __QWS_MANAGER_H__
#define __QWS_MANAGER_H__

#ifndef QT_H
#include "qpixmap.h"
#include "qobject.h"
#endif // QT_H

#ifndef QT_NO_QWS_MANAGER

class QPixmap;
class QWidget;
class QPopupMenu;
class QRegion;
class QMouseEvent;
class QWSButton;
class QWSManager;

/*
 Implements decoration styles
*/
class QWSDecoration
{
public:
    QWSDecoration() {}
    virtual ~QWSDecoration() {}

    enum Region { None=0, All=1, Title=2, Top=3, Bottom=4, Left=5, Right=6,
		TopLeft=7, TopRight=8, BottomLeft=9, BottomRight=10,
		Close=11, Minimize=12, Maximize=13, Normalize=14,
		Menu=15, LastRegion=Menu };

    virtual QRegion region(const QWidget *, const QRect &rect, Region r=All) = 0;
    virtual void close( QWidget * );
    virtual void minimize( QWidget * );
    virtual void maximize( QWidget * );
#ifndef QT_NO_POPUPMENU
    virtual QPopupMenu *menu(QWSManager *, const QWidget *, const QPoint &);
#endif
    virtual void paint(QPainter *, const QWidget *) = 0;
    virtual void paintButton(QPainter *, const QWidget *, Region, int state) = 0;
};


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
};

class QWSButton
{
public:
    QWSButton(QWSManager *m, QWSDecoration::Region t, bool tb = false);

    enum State { MouseOver = 0x01, Clicked = 0x02, On = 0x04 };
    int state() { return flags; }
    void setMouseOver(bool);
    void setClicked(bool);
    void setOn(bool);

protected:
    void paint();

private:
    int  flags;
    bool toggle;
    QWSDecoration::Region type;
    QWSManager *manager;
};

// class QWSDefaultDecoration : public QWSDecoration;
#include "qwsdefaultdecoration_qws.h"

#endif //QT_NO_QWS_MANAGER

#endif //__QWS_MANAGER_H__
